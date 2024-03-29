// Emacs style mode select   -*- C -*-
//-----------------------------------------------------------------------------
//
// Copyright(C) 2009 Simon Howard
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
// 02111-1307, USA.
//
// DESCRIPTION:
//     OPL interface.
//
//-----------------------------------------------------------------------------

#include "config.h"

#include <stdio.h>
#include <stdlib.h>

#include "opl3.h"
#include "opl.h"
#include "opl_queue.h"

#include "dsda/configuration.h"

static int init_stage_reg_writes = 1;

unsigned int opl_sample_rate = 22050;

#define MAX_SOUND_SLICE_TIME 100 /* ms */

typedef struct
{
    unsigned int rate;        // Number of times the timer is advanced per sec.
    unsigned int enabled;     // Non-zero if timer is enabled.
    unsigned int value;       // Last value that was set.
    uint64_t expire_time;     // Calculated time that timer will expire.
} opl_timer_t;

// Queue of callbacks waiting to be invoked.
static opl_callback_queue_t *callback_queue;

// Current time, in us since startup:
static uint64_t current_time;

// If non-zero, playback is currently paused.
static int opl_paused;

// Time offset (in us) due to the fact that callbacks
// were previously paused.
static uint64_t pause_offset;

// OPL software emulator structure.
static opl3_chip opl_chip;
static int opl_opl3mode;

// Register number that was written.
static int register_num = 0;

// Timers; DBOPL does not do timer stuff itself.
static opl_timer_t timer1 = { 12500, 0, 0, 0 };
static opl_timer_t timer2 = { 3125, 0, 0, 0 };

// Initialize the OPL library.  Returns true if initialized
// successfully.
int OPL_Init (unsigned int rate)
{
    opl_opl3mode = dsda_IntConfig(dsda_config_mus_opl_opl3mode);

    opl_sample_rate = rate;
    opl_paused = 0;
    pause_offset = 0;

    // Queue structure of callbacks to invoke.

    callback_queue = OPL_Queue_Create();
    current_time = 0;

    // Create the emulator structure:
    OPL3_Reset(&opl_chip, opl_sample_rate);

    OPL_InitRegisters(opl_opl3mode);

    init_stage_reg_writes = 0;

    return 1;
}

// Shut down the OPL library.
void OPL_Shutdown(void)
{
    if (callback_queue)
    {
      OPL_Queue_Destroy(callback_queue);

      callback_queue = NULL;
    }
}

void OPL_SetCallback(uint64_t us,
                     opl_callback_t callback,
                     void *data)
{
    OPL_Queue_Push(callback_queue, callback, data,
                   current_time - pause_offset + us);
}

void OPL_ClearCallbacks(void)
{
    OPL_Queue_Clear(callback_queue);
}

static void OPLTimer_CalculateEndTime(opl_timer_t *timer)
{
    int tics;

    // If the timer is enabled, calculate the time when the timer
    // will expire.
    if (timer->enabled)
    {
        tics = 0x100 - timer->value;
        timer->expire_time = current_time
                           + ((uint64_t) tics * OPL_SECOND) / timer->rate;
    }
}

static void WriteRegister(unsigned int reg_num, unsigned int value)
{
    switch (reg_num)
    {
        case OPL_REG_TIMER1:
            timer1.value = value;
            OPLTimer_CalculateEndTime(&timer1);
            break;

        case OPL_REG_TIMER2:
            timer2.value = value;
            OPLTimer_CalculateEndTime(&timer2);
            break;

        case OPL_REG_TIMER_CTRL:
            if (value & 0x80)
            {
                timer1.enabled = 0;
                timer2.enabled = 0;
            }
            else
            {
                if ((value & 0x40) == 0)
                {
                    timer1.enabled = (value & 0x01) != 0;
                    OPLTimer_CalculateEndTime(&timer1);
                }

                if ((value & 0x20) == 0)
                {
                    timer1.enabled = (value & 0x02) != 0;
                    OPLTimer_CalculateEndTime(&timer2);
                }
            }

            break;

        case OPL_REG_NEW:
            opl_opl3mode = value & 0x01;

        default:
            OPL3_WriteRegBuffered(&opl_chip, reg_num, value);
            break;
    }
}

static void OPL_AdvanceTime(unsigned int nsamples)
{
    opl_callback_t callback;
    void *callback_data;
    uint64_t us;

    // Advance time.
    us = ((uint64_t) nsamples * OPL_SECOND) / opl_sample_rate;
    current_time += us;

    if (opl_paused)
    {
        pause_offset += us;
    }

    // Are there callbacks to invoke now?  Keep invoking them
    // until there are none more left.
    while (!OPL_Queue_IsEmpty(callback_queue)
        && current_time >= OPL_Queue_Peek(callback_queue) + pause_offset)
    {
        // Pop the callback from the queue to invoke it.
        if (!OPL_Queue_Pop(callback_queue, &callback, &callback_data))
        {
            break;
        }

        callback(callback_data);
    }
}

void OPL_AdjustCallbacks(float factor)
{
    OPL_Queue_AdjustCallbacks(callback_queue, current_time, factor);
}

void OPL_Render_Samples (void *dest, unsigned buffer_len)
{
    unsigned int filled = 0;
    int16_t *buffer = (int16_t *) dest;

    // Repeatedly call the OPL emulator update function until the buffer is
    // full.
    while (filled < buffer_len)
    {
        uint64_t next_callback_time;
        uint64_t nsamples;

        // Work out the time until the next callback waiting in
        // the callback queue must be invoked.  We can then fill the
        // buffer with this many samples.
        if (opl_paused || OPL_Queue_IsEmpty(callback_queue))
        {
            nsamples = buffer_len - filled;
        }
        else
        {
            next_callback_time = OPL_Queue_Peek(callback_queue) + pause_offset;

            nsamples = (next_callback_time - current_time) * opl_sample_rate;
            nsamples = (nsamples + OPL_SECOND - 1) / OPL_SECOND;

            if (nsamples > buffer_len - filled)
            {
                nsamples = buffer_len - filled;
            }
        }

        // Add emulator output to buffer.
        OPL3_GenerateStream(&opl_chip, buffer + filled * 2, nsamples);
        filled += nsamples;

        // Invoke callbacks for this point in time.
        OPL_AdvanceTime(nsamples);
    }
}

void OPL_WritePort(opl_port_t port, unsigned int value)
{
    if (port == OPL_REGISTER_PORT)
    {
        register_num = value;
    }
    else if (port == OPL_REGISTER_PORT_OPL3)
    {
        register_num = value | 0x100;
    }
    else if (port == OPL_DATA_PORT)
    {
        WriteRegister(register_num, value);
    }
}

unsigned int OPL_ReadPort(opl_port_t port)
{
    unsigned int result = 0;

    if (port == OPL_REGISTER_PORT_OPL3)
    {
        return 0xff;
    }

    if (timer1.enabled && current_time > timer1.expire_time)
    {
        result |= 0x80;   // Either have expired
        result |= 0x40;   // Timer 1 has expired
    }

    if (timer2.enabled && current_time > timer2.expire_time)
    {
        result |= 0x80;   // Either have expired
        result |= 0x20;   // Timer 2 has expired
    }

    return result;
}

//
// Higher-level functions, based on the lower-level functions above
// (register write, etc).
//
unsigned int OPL_ReadStatus(void)
{
    return OPL_ReadPort(OPL_REGISTER_PORT);
}

// Write an OPL register value
void OPL_WriteRegister(int reg, int value)
{
    int i;

    if (reg & 0x100)
    {
        OPL_WritePort(OPL_REGISTER_PORT_OPL3, reg);
    }
    else
    {
        OPL_WritePort(OPL_REGISTER_PORT, reg);
    }

    // For timing, read the register port six times after writing the
    // register number to cause the appropriate delay
    for (i=0; i<6; ++i)
    {
        // An oddity of the Doom OPL code: at startup initialization,
        // the spacing here is performed by reading from the register
        // port; after initialization, the data port is read, instead.
        if (init_stage_reg_writes)
        {
            OPL_ReadPort(OPL_REGISTER_PORT);
        }
        else
        {
            OPL_ReadPort(OPL_DATA_PORT);
        }
    }

    OPL_WritePort(OPL_DATA_PORT, value);

    // Read the register port 24 times after writing the value to
    // cause the appropriate delay
    for (i=0; i<24; ++i)
    {
        OPL_ReadStatus();
    }
}

// Initialize registers on startup
void OPL_InitRegisters(int opl3)
{
    int r;

    // Initialize level registers
    for (r=OPL_REGS_LEVEL; r <= OPL_REGS_LEVEL + OPL_NUM_OPERATORS; ++r)
    {
        OPL_WriteRegister(r, 0x3f);
    }

    // Initialize other registers
    // These two loops write to registers that actually don't exist,
    // but this is what Doom does ...
    // Similarly, the <= is also intenational.
    for (r=OPL_REGS_ATTACK; r <= OPL_REGS_WAVEFORM + OPL_NUM_OPERATORS; ++r)
    {
        OPL_WriteRegister(r, 0x00);
    }

    // More registers ...
    for (r=1; r < OPL_REGS_LEVEL; ++r)
    {
        OPL_WriteRegister(r, 0x00);
    }

    // Re-initialize the low registers:

    // Reset both timers and enable interrupts:
    OPL_WriteRegister(OPL_REG_TIMER_CTRL,      0x60);
    OPL_WriteRegister(OPL_REG_TIMER_CTRL,      0x80);

    // "Allow FM chips to control the waveform of each operator":
    OPL_WriteRegister(OPL_REG_WAVEFORM_ENABLE, 0x20);

    if (opl3)
    {
        OPL_WriteRegister(OPL_REG_NEW, 0x01);

        // Initialize level registers
        for (r=OPL_REGS_LEVEL; r <= OPL_REGS_LEVEL + OPL_NUM_OPERATORS; ++r)
        {
            OPL_WriteRegister(r | 0x100, 0x3f);
        }

        // Initialize other registers
        // These two loops write to registers that actually don't exist,
        // but this is what Doom does ...
        // Similarly, the <= is also intenational.
        for (r=OPL_REGS_ATTACK; r <= OPL_REGS_WAVEFORM + OPL_NUM_OPERATORS; ++r)
        {
            OPL_WriteRegister(r | 0x100, 0x00);
        }

        // More registers ...
        for (r=1; r < OPL_REGS_LEVEL; ++r)
        {
            OPL_WriteRegister(r | 0x100, 0x00);
        }
    }

    // Keyboard split point on (?)
    OPL_WriteRegister(OPL_REG_FM_MODE,         0x40);

    if (opl3)
    {
        OPL_WriteRegister(OPL_REG_NEW, 0x01);
    }
}

void OPL_SetPaused(int paused)
{
    opl_paused = paused;
}
