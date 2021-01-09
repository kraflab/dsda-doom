
/*
===================
=
= G_CheckDemoStatus
=
= Called after a death or level completion to allow demos to be cleaned up
= Returns true if a new demo loop action will take place
===================
*/

boolean G_CheckDemoStatus(void)
{
    int endtime, realtics;

    if (timingdemo)
    {
        float fps;
        endtime = I_GetTime();
        realtics = endtime - starttime;
        fps = ((float) gametic * TICRATE) / realtics;
        I_Error("timed %i gametics in %i realtics (%f fps)",
                gametic, realtics, fps);
    }

    if (demoplayback)
    {
        if (singledemo)
            I_Quit();

        W_ReleaseLumpName(defdemoname);
        demoplayback = false;
        D_AdvanceDemo();
        return true;
    }

    if (demorecording)
    {
        *demo_p++ = DEMOMARKER;
        M_WriteFile(demoname, demobuffer, demo_p - demobuffer);
        Z_Free(demobuffer);
        demorecording = false;
        // [crispy] if a new game is started during demo recording, start a new demo
        if (gameaction != ga_newgame)
        {
        I_Error("Demo %s recorded", demoname);
        }
        else
        {
            fprintf(stderr, "Demo %s recorded\n", demoname);
        }
    }

    return false;
}
