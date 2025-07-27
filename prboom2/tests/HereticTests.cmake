include_guard()

function(add_heretic_test lump_path expected_time extra_args)
  string(REPLACE "/" "_" lump_name ${lump_path})
  add_test(
    NAME dsda.heretic.${lump_name}
    COMMAND ${CMAKE_COMMAND}
      "-DDSDA_DOOM_EXECUTABLE=${dsda_executable_arg}"
      -DTEST_LUMP=heretic/${lump_path}.lmp
      -DEXPECTED_FINAL_TIME=${expected_time}
      -DTEST_EXTRA_ARGS=${extra_args}
      -P ${CMAKE_CURRENT_LIST_DIR}/HereticTestRunner.cmake
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/heretic/${lump_name}"
  )
  file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/heretic/${lump_name}")
  set_tests_properties(dsda.heretic.${lump_name}
    PROPERTIES
    LABELS heretic
  )
endfunction()

add_heretic_test(26/h1f1-130 1:30 "") # heretic E1M1 BP Max in 1:30 by Hitherto
add_heretic_test(27/h1f1-133 1:33 "") # heretic E1M1 BP Max in 1:33 by Xit Vono
add_heretic_test(28/h1f1-135 1:35 "") # heretic E1M1 BP Max in 1:35 by Hitherto
add_heretic_test(29/h1f1-138 1:38 "") # heretic E1M1 BP Max in 1:38 by Xit Vono
add_heretic_test(30/h1f1-141 1:41 "") # heretic E1M1 BP Max in 1:41 by Hitherto
add_heretic_test(451/h1f1-214 2:14 "") # heretic E1M1 BP Max in 2:14 by Vincent Catalaa
add_heretic_test(74746/11bpm_219 2:18 "") # heretic E1M1 BP Max in 2:18 by LadyMistDragon
add_heretic_test(615/hs5e1m1 5:32 "") # heretic E1M1 BP Max in 5:32 by Aleksey Karakin
add_heretic_test(51475/h1b1x025 0:25 "") # heretic E1M1 BP Speed in 0:25 by Dimon12321
add_heretic_test(59974/h1b1-026 0:26 "") # heretic E1M1 BP Speed in 0:26 by Mikolah
add_heretic_test(59492/h1b1-027 0:27 "") # heretic E1M1 BP Speed in 0:27 by Mikolah
add_heretic_test(3/h1b1-028 0:28 "") # heretic E1M1 BP Speed in 0:28 by Vincent Catalaa
add_heretic_test(4/h1b1-032 0:32 "") # heretic E1M1 BP Speed in 0:32 by Vincent Catalaa
add_heretic_test(74758/11bps_3743 0:37 "") # heretic E1M1 BP Speed in 0:37 by LadyMistDragon
add_heretic_test(73288/h1x1-041 0:41 "") # heretic E1M1 NM 100S in 0:41 by Insomnia
add_heretic_test(113/h1x1-045 0:45 "") # heretic E1M1 NM 100S in 0:45 by JCD
add_heretic_test(74747/11bp_nm100s-00011_10551 1:05 "") # heretic E1M1 NM 100S in 1:05 by LadyMistDragon
add_heretic_test(75934/h1n1-026 0:26 "") # heretic E1M1 NM Speed in 0:26 by Insomnia
add_heretic_test(72917/h1n1-2760 0:27 "") # heretic E1M1 NM Speed in 0:27 by Phoenix
add_heretic_test(36/h1n1-027 0:27 "") # heretic E1M1 NM Speed in 0:27 by JCD
add_heretic_test(37/h1n1-028 0:28 "") # heretic E1M1 NM Speed in 0:28 by Vincent Catalaa
add_heretic_test(38/h1n1-030 0:30 "") # heretic E1M1 NM Speed in 0:30 by JCD
add_heretic_test(600/11nm 1:11 "-respawn") # heretic E1M1 NM Speed in 1:11 by Anonymous
add_heretic_test(681/h1c1trik 0:14 "") # heretic E1M1 NoMo in 0:14 by Michael Lastovica, Vincent Catalaa
add_heretic_test(51786/h1o12463 0:24 "") # heretic E1M1 NoMo in 0:24 by depr4vity
add_heretic_test(47536/h1o12491 0:24 "") # heretic E1M1 NoMo in 0:24 by depr4vity
add_heretic_test(52/h1o12568 0:25 "") # heretic E1M1 NoMo in 0:25 by Kimo Xvirus
add_heretic_test(62592/h1s1o-2597 0:25 "") # heretic E1M1 NoMo in 0:25 by AmnesiaSilence
add_heretic_test(682/h1o12637 0:26 "") # heretic E1M1 NoMo in 0:26 by Kimo Xvirus
add_heretic_test(51/h1o12682 0:26 "-nomonsters") # heretic E1M1 NoMo in 0:26 by Vincent Catalaa
add_heretic_test(815/h1o1-026 0:26 "") # heretic E1M1 NoMo in 0:26 by Batawi
add_heretic_test(74181/o1s1-044 0:44 "") # heretic E1M1 NoMo 100S in 0:44 by El Tom
add_heretic_test(65/h1p1-027 0:27 "") # heretic E1M1 Pacifist in 0:27 by Kimo Xvirus
add_heretic_test(66/h1p1-030 0:30 "") # heretic E1M1 Pacifist in 0:30 by JCD
add_heretic_test(63017/c1m1x027 0:27 "") # heretic E1M1 SM Max in 0:27 by JCD
add_heretic_test(436/c1m1j113 1:13 "") # heretic E1M1 SM Max in 1:13 by JCD, Ch0wW
add_heretic_test(68075/h1m1-131 1:31 "") # heretic E1M1 SM Max in 1:31 by Bainshie-Doom
add_heretic_test(32/h1m1-133 1:33 "") # heretic E1M1 SM Max in 1:33 by Kimo Xvirus
add_heretic_test(33/h1m1-206 2:06 "") # heretic E1M1 SM Max in 2:06 by JCD
add_heretic_test(34/h1m1-238 2:38 "") # heretic E1M1 SM Max in 2:38 by William Huber
add_heretic_test(463/h1m1-400 4:00 "") # heretic E1M1 SM Max in 4:00 by Arno Slagboom
add_heretic_test(676/htuve1m1 5:09 "") # heretic E1M1 SM Max in 5:09 by Dave Turner
add_heretic_test(437/h1c1c011 0:11 "") # heretic E1M1 SM Speed in 0:11 by JCD, Ch0wW
add_heretic_test(1/h1s1-025 0:25 "") # heretic E1M1 SM Speed in 0:25 by Kimo Xvirus
add_heretic_test(81/h1s1-026 0:26 "") # heretic E1M1 SM Speed in 0:26 by Kimo Xvirus
add_heretic_test(438/h1c1-027 0:27 "") # heretic E1M1 SM Speed in 0:27 by JCD, Suicider
add_heretic_test(82/h1s1-027 0:27 "") # heretic E1M1 SM Speed in 0:27 by Vincent Catalaa
add_heretic_test(62591/h1s1-2871 0:28 "") # heretic E1M1 SM Speed in 0:28 by AmnesiaSilence
add_heretic_test(83/h1s1-028 0:28 "") # heretic E1M1 SM Speed in 0:28 by Radek Pecka
add_heretic_test(84/h1s1-029 0:29 "") # heretic E1M1 SM Speed in 0:29 by Vincent Catalaa
add_heretic_test(620/h1s1a033 0:33 "") # heretic E1M1 SM Speed in 0:33 by Arno Slagboom
add_heretic_test(53452/h1s1-34 0:34 "") # heretic E1M1 SM Speed in 0:34 by xX_Lol6_Xx
add_heretic_test(75488/h1t1-149 1:49 "") # heretic E1M1 Tyson in 1:49 by Insomnia
add_heretic_test(109/h1t1-152 1:52 "") # heretic E1M1 Tyson in 1:52 by JCD
add_heretic_test(482/h1t1-159 1:59 "") # heretic E1M1 Tyson in 1:59 by Hitherto
add_heretic_test(110/h1t1-203 2:03 "") # heretic E1M1 Tyson in 2:03 by JCD
add_heretic_test(483/h1t1-214 2:14 "") # heretic E1M1 Tyson in 2:14 by Hitherto
add_heretic_test(111/h1t1-240 2:40 "") # heretic E1M1 Tyson in 2:40 by Xit Vono
add_heretic_test(74694/h1f2-353 3:53 "") # heretic E1M2 BP Max in 3:53 by Doomniel
add_heretic_test(73447/h1f2-412 4:12 "") # heretic E1M2 BP Max in 4:12 by Doomniel
add_heretic_test(452/h1f2-422 4:22 "") # heretic E1M2 BP Max in 4:22 by Hitherto
add_heretic_test(453/h1f2-511 5:11 "") # heretic E1M2 BP Max in 5:11 by Xit Vono
add_heretic_test(454/h1f2-523 5:23 "") # heretic E1M2 BP Max in 5:23 by Laurent Sebellin
add_heretic_test(616/hs5e1m2 8:25 "") # heretic E1M2 BP Max in 8:25 by Aleksey Karakin
add_heretic_test(75905/lmd_12bpm1 9:43 "") # heretic E1M2 BP Max in 9:43 by LadyMistDragon
add_heretic_test(60096/heretic-e1m2b-11023 1:10 "") # heretic E1M2 BP Speed in 1:10 by Ks4
add_heretic_test(5/h1b2-115 1:15 "") # heretic E1M2 BP Speed in 1:15 by Xit Vono
add_heretic_test(6/h1b2-118 1:18 "") # heretic E1M2 BP Speed in 1:18 by QWERTY
add_heretic_test(7/h1b2-126 1:26 "") # heretic E1M2 BP Speed in 1:26 by QWERTY
add_heretic_test(8/h1b2-134 1:34 "") # heretic E1M2 BP Speed in 1:34 by QWERTY
add_heretic_test(9/h1b2-153 1:53 "") # heretic E1M2 BP Speed in 1:53 by Vincent Catalaa
add_heretic_test(73289/h1x2-139 1:39 "") # heretic E1M2 NM 100S in 1:39 by Insomnia
add_heretic_test(51476/h1x2-140 1:40 "") # heretic E1M2 NM 100S in 1:40 by JCD
add_heretic_test(75906/lmd_12_100s319 3:07 "") # heretic E1M2 NM 100S in 3:07 by LadyMistDragon
add_heretic_test(40/h1n2-143 1:43 "") # heretic E1M2 NM Speed in 1:43 by JCD
add_heretic_test(600/12nm 3:34 "-respawn") # heretic E1M2 NM Speed in 3:34 by Jeff N. Easthope
add_heretic_test(76564/h1o25851 0:58 "") # heretic E1M2 NoMo in 0:58 by Ks4
add_heretic_test(73882/h1o2-058 0:58 "") # heretic E1M2 NoMo in 0:58 by Insomnia
add_heretic_test(53/h1o2-103 1:03 "") # heretic E1M2 NoMo in 1:03 by Kimo Xvirus
add_heretic_test(54/h1o2-107 1:07 "-nomonsters") # heretic E1M2 NoMo in 1:07 by Vincent Catalaa
add_heretic_test(74182/o1s2-132 1:32 "") # heretic E1M2 NoMo 100S in 1:32 by El Tom
add_heretic_test(55770/h1p2-157 1:57 "") # heretic E1M2 Pacifist in 1:57 by 4shockblast
add_heretic_test(479/h1p2-315 3:15 "") # heretic E1M2 Pacifist in 3:15 by Xit Vono
add_heretic_test(72567/h1m2-332 3:32 "") # heretic E1M2 SM Max in 3:32 by Insomnia
add_heretic_test(464/h1m2-422 4:22 "") # heretic E1M2 SM Max in 4:22 by Hitherto
add_heretic_test(465/h1m2-551 5:51 "") # heretic E1M2 SM Max in 5:51 by Vincent Catalaa
add_heretic_test(2/h1s2-110 1:10 "") # heretic E1M2 SM Speed in 1:10 by Kimo Xvirus
add_heretic_test(85/h1s2-118 1:18 "") # heretic E1M2 SM Speed in 1:18 by JCD
add_heretic_test(86/h1s2-121 1:21 "") # heretic E1M2 SM Speed in 1:21 by Xit Vono
add_heretic_test(87/h1s2-122 1:22 "") # heretic E1M2 SM Speed in 1:22 by Vincent Catalaa
add_heretic_test(88/h1s2-134 1:34 "") # heretic E1M2 SM Speed in 1:34 by QWERTY
add_heretic_test(89/h1s2-136 1:36 "") # heretic E1M2 SM Speed in 1:36 by Radek Pecka
add_heretic_test(90/h1s2-140 1:40 "") # heretic E1M2 SM Speed in 1:40 by Vincent Catalaa
add_heretic_test(628/h1s2v223 2:22 "") # heretic E1M2 SM Speed in 2:22 by Vincent Catalaa
add_heretic_test(677/htuve1m2 9:50 "") # heretic E1M2 SM Speed in 9:50 by Dave Turner
add_heretic_test(484/h1t2-809 8:09 "") # heretic E1M2 Tyson in 8:09 by Xit Vono
add_heretic_test(455/h1f3-241 2:41 "") # heretic E1M3 BP Max in 2:41 by Vile
add_heretic_test(456/h1f3-250 2:50 "") # heretic E1M3 BP Max in 2:50 by Vile
add_heretic_test(54166/heretice1m3b-4334 0:43 "") # heretic E1M3 BP Speed in 0:43 by Ks4
add_heretic_test(10/h1b3-046 0:46 "") # heretic E1M3 BP Speed in 0:46 by QWERTY
add_heretic_test(11/h1b3-048 0:48 "") # heretic E1M3 BP Speed in 0:48 by QWERTY
add_heretic_test(12/h1b3-050 0:50 "") # heretic E1M3 BP Speed in 0:50 by Vincent Catalaa
add_heretic_test(73290/h1x3-111 1:11 "") # heretic E1M3 NM 100S in 1:11 by Insomnia
add_heretic_test(51477/h1x3-120 1:20 "") # heretic E1M3 NM 100S in 1:20 by JCD
add_heretic_test(73881/h1n3-047 0:47 "") # heretic E1M3 NM Speed in 0:47 by El Tom
add_heretic_test(41/h1n3-054 0:54 "") # heretic E1M3 NM Speed in 0:54 by JCD
add_heretic_test(600/13nm 3:18 "-respawn") # heretic E1M3 NM Speed in 3:18 by Anonymous
add_heretic_test(73883/h1o3-035 0:35 "") # heretic E1M3 NoMo in 0:35 by Insomnia
add_heretic_test(55/h1o33674 0:36 "-nomonsters") # heretic E1M3 NoMo in 0:36 by Vincent Catalaa
add_heretic_test(817/h1o3-037 0:37 "") # heretic E1M3 NoMo in 0:37 by Batawi
add_heretic_test(74183/o1s3-104 1:04 "") # heretic E1M3 NoMo 100S in 1:04 by El Tom
add_heretic_test(74455/h1p3-041 0:41 "") # heretic E1M3 Pacifist in 0:41 by Insomnia
add_heretic_test(67/h1p3-101 1:01 "") # heretic E1M3 Pacifist in 1:01 by JCD
add_heretic_test(72568/h1m3-413 4:13 "") # heretic E1M3 SM Max in 4:13 by Insomnia
add_heretic_test(466/h1m3-509 5:09 "") # heretic E1M3 SM Max in 5:09 by Vincent Catalaa
add_heretic_test(47544/h1s3-039 0:39 "") # heretic E1M3 SM Speed in 0:39 by depr4vity
add_heretic_test(91/h1s3-045 0:45 "") # heretic E1M3 SM Speed in 0:45 by Vincent Catalaa
add_heretic_test(92/h1s3-048 0:48 "") # heretic E1M3 SM Speed in 0:48 by Radek Pecka
add_heretic_test(93/h1s3-049 0:49 "") # heretic E1M3 SM Speed in 0:49 by Vincent Catalaa
add_heretic_test(678/htuve1m3 10:06 "") # heretic E1M3 SM Speed in 10:06 by Dave Turner
add_heretic_test(630/h1s3v110 1:09 "") # heretic E1M3 SM Speed in 1:09 by Vincent Catalaa
add_heretic_test(485/h1t3-630 6:30 "") # heretic E1M3 Tyson in 6:30 by Xit Vono
add_heretic_test(457/h1f4-310 3:10 "") # heretic E1M4 BP Max in 3:10 by Kimo Xvirus
add_heretic_test(458/h1f4-427 4:27 "") # heretic E1M4 BP Max in 4:27 by Vincent Catalaa
add_heretic_test(61095/h1-4b-3897 0:38 "") # heretic E1M4 BP Speed in 0:38 by Ks4
add_heretic_test(60097/heretic-e1m4b-4011 0:40 "") # heretic E1M4 BP Speed in 0:40 by Ks4
add_heretic_test(54167/heretice1m4b-4360 0:43 "") # heretic E1M4 BP Speed in 0:43 by Ks4
add_heretic_test(13/h1b4-048 0:48 "") # heretic E1M4 BP Speed in 0:48 by QWERTY
add_heretic_test(14/h1b4-049 0:49 "") # heretic E1M4 BP Speed in 0:49 by Vincent Catalaa
add_heretic_test(73291/h1x4-053 0:53 "") # heretic E1M4 NM 100S in 0:53 by Insomnia
add_heretic_test(51478/h1x4-054 0:54 "") # heretic E1M4 NM 100S in 0:54 by JCD
add_heretic_test(75935/h1n4-041 0:41 "") # heretic E1M4 NM Speed in 0:41 by Insomnia
add_heretic_test(42/h1n4-059 0:59 "") # heretic E1M4 NM Speed in 0:59 by JCD
add_heretic_test(600/14nm 2:52 "-respawn") # heretic E1M4 NM Speed in 2:52 by Anonymous
add_heretic_test(76281/h1o43029 0:30 "") # heretic E1M4 NoMo in 0:30 by Ks4
add_heretic_test(73596/h1o4-030 0:30 "") # heretic E1M4 NoMo in 0:30 by Insomnia
add_heretic_test(56/h1o43105 0:31 "-nomonsters") # heretic E1M4 NoMo in 0:31 by Vincent Catalaa
add_heretic_test(818/h1o4-031 0:31 "") # heretic E1M4 NoMo in 0:31 by Batawi
add_heretic_test(74184/o1s4-052 0:52 "") # heretic E1M4 NoMo 100S in 0:52 by El Tom
add_heretic_test(68/h1p4-156 1:56 "") # heretic E1M4 Pacifist in 1:56 by Xit Vono
add_heretic_test(72569/h1m4-311 3:11 "") # heretic E1M4 SM Max in 3:11 by Insomnia
add_heretic_test(55877/h1m4-418 4:18 "") # heretic E1M4 SM Max in 4:18 by Archi
add_heretic_test(467/h1m4-532 5:32 "") # heretic E1M4 SM Max in 5:32 by Vincent Catalaa
add_heretic_test(75837/h1s4-038 0:38 "") # heretic E1M4 SM Speed in 0:38 by Insomnia
add_heretic_test(61094/h1-4-3889 0:38 "") # heretic E1M4 SM Speed in 0:38 by Ks4
add_heretic_test(94/h1s4-039 0:39 "") # heretic E1M4 SM Speed in 0:39 by Kimo Xvirus
add_heretic_test(95/h1s4-045 0:45 "") # heretic E1M4 SM Speed in 0:45 by Vincent Catalaa
add_heretic_test(631/h1s4v139 1:38 "") # heretic E1M4 SM Speed in 1:38 by Vincent Catalaa
add_heretic_test(679/htuve1m4 7:37 "") # heretic E1M4 SM Speed in 7:37 by Dave Turner
add_heretic_test(486/h1t4-522 5:22 "") # heretic E1M4 Tyson in 5:22 by Kimo Xvirus
add_heretic_test(684/h1t4-527 5:27 "") # heretic E1M4 Tyson in 5:27 by Hitherto
add_heretic_test(459/h1f5-613 6:13 "") # heretic E1M5 BP Max in 6:13 by Vincent Catalaa
add_heretic_test(60099/heretic-e1m5b-4789 0:47 "") # heretic E1M5 BP Speed in 0:47 by Ks4
add_heretic_test(15/h1b5055n 0:55 "") # heretic E1M5 BP Speed in 0:55 by QWERTY
add_heretic_test(16/h1b5-101 1:01 "") # heretic E1M5 BP Speed in 1:01 by Vincent Catalaa
add_heretic_test(73414/h1x5-110 1:10 "") # heretic E1M5 NM 100S in 1:10 by Insomnia
add_heretic_test(51479/h1x5-115 1:15 "") # heretic E1M5 NM 100S in 1:15 by JCD
add_heretic_test(43/h1n5-056 0:56 "") # heretic E1M5 NM Speed in 0:56 by JCD
add_heretic_test(600/15nm 3:10 "-respawn") # heretic E1M5 NM Speed in 3:10 by Jeff N. Easthope
add_heretic_test(73884/h1o5-036 0:36 "") # heretic E1M5 NoMo in 0:36 by Insomnia
add_heretic_test(57/h1o53934 0:39 "-nomonsters") # heretic E1M5 NoMo in 0:39 by Vincent Catalaa
add_heretic_test(819/h1o5-039 0:39 "") # heretic E1M5 NoMo in 0:39 by Batawi
add_heretic_test(74185/o1s5-101 1:01 "") # heretic E1M5 NoMo 100S in 1:01 by El Tom
add_heretic_test(675/htnme1m5 5:28 "-nomonsters") # heretic E1M5 NoMo 100S in 5:28 by Dave Turner
add_heretic_test(69/h1p5-149 1:49 "") # heretic E1M5 Pacifist in 1:49 by Xit Vono
add_heretic_test(72570/h1m5-426 4:26 "") # heretic E1M5 SM Max in 4:26 by Insomnia
add_heretic_test(468/h1m5-637 6:37 "") # heretic E1M5 SM Max in 6:37 by Vincent Catalaa
add_heretic_test(75838/h1s5-047 0:47 "") # heretic E1M5 SM Speed in 0:47 by Insomnia
add_heretic_test(60098/heretic-e1m5-5051 0:50 "") # heretic E1M5 SM Speed in 0:50 by Ks4
add_heretic_test(96/h1s5-052 0:52 "") # heretic E1M5 SM Speed in 0:52 by Vincent Catalaa
add_heretic_test(632/h1s5v146 1:45 "") # heretic E1M5 SM Speed in 1:45 by Vincent Catalaa
add_heretic_test(617/htyde1m5 2:09 "") # heretic E1M5 SM Speed in 2:09 by Dave Turner
add_heretic_test(75489/h1t5-613 6:13 "") # heretic E1M5 Tyson in 6:13 by Insomnia
add_heretic_test(460/h1f6-618 6:18 "") # heretic E1M6 BP Max in 6:18 by William Huber
add_heretic_test(54169/heretice1m6b-10751 1:07 "") # heretic E1M6 BP Speed in 1:07 by Ks4
add_heretic_test(17/h1b6-111 1:11 "") # heretic E1M6 BP Speed in 1:11 by QWERTY
add_heretic_test(18/h1b6-125 1:25 "") # heretic E1M6 BP Speed in 1:25 by Vincent Catalaa
add_heretic_test(73292/h1x6-134 1:34 "") # heretic E1M6 NM 100S in 1:34 by Insomnia
add_heretic_test(51480/h1x6-141 1:41 "") # heretic E1M6 NM 100S in 1:41 by JCD
add_heretic_test(44/h1n6-120 1:20 "") # heretic E1M6 NM Speed in 1:20 by JCD
add_heretic_test(600/16nm 2:28 "-respawn") # heretic E1M6 NM Speed in 2:28 by Jeff N. Easthope
add_heretic_test(76565/h1o6-049 0:49 "") # heretic E1M6 NoMo in 0:49 by Insomnia
add_heretic_test(820/h1o6-051 0:51 "") # heretic E1M6 NoMo in 0:51 by Batawi
add_heretic_test(58/h1o65231 0:52 "-nomonsters") # heretic E1M6 NoMo in 0:52 by Vincent Catalaa
add_heretic_test(74186/o1s6-128 1:28 "") # heretic E1M6 NoMo 100S in 1:28 by El Tom
add_heretic_test(50727/o1s6-129 1:29 "") # heretic E1M6 NoMo 100S in 1:29 by PVS
add_heretic_test(70/h1p6-209 2:09 "") # heretic E1M6 Pacifist in 2:09 by Lucas Marincak
add_heretic_test(71/h1p6-228 2:28 "") # heretic E1M6 Pacifist in 2:28 by JCD
add_heretic_test(480/h1p6-442 4:42 "") # heretic E1M6 Pacifist in 4:42 by Xit Vono
add_heretic_test(618/htuve1m6 16:30 "") # heretic E1M6 SM Max in 16:30 by Dave Turner
add_heretic_test(72571/h1m6-445 4:45 "") # heretic E1M6 SM Max in 4:45 by Insomnia
add_heretic_test(469/h1m6-640 6:40 "") # heretic E1M6 SM Max in 6:40 by Vincent Catalaa
add_heretic_test(63412/h1s6-058 0:58 "") # heretic E1M6 SM Speed in 0:58 by JCD
add_heretic_test(54168/heretice1m6-10683 1:06 "") # heretic E1M6 SM Speed in 1:06 by Ks4
add_heretic_test(97/h1s6-110 1:10 "") # heretic E1M6 SM Speed in 1:10 by Vincent Catalaa
add_heretic_test(75490/h1t6-916 9:41 "") # heretic E1M6 Tyson in 9:41 by Insomnia
add_heretic_test(54171/heretice1m6sb-11269 1:12 "") # heretic E1M6s BP Speed in 1:12 by Ks4
add_heretic_test(19/h1b6s134 1:34 "") # heretic E1M6s BP Speed in 1:34 by Vincent Catalaa
add_heretic_test(45/h1n6s112 1:12 "") # heretic E1M6s NM Speed in 1:12 by Vincent Catalaa
add_heretic_test(76566/h1o6s-053 0:53 "") # heretic E1M6s NoMo in 0:53 by Insomnia
add_heretic_test(821/h1o6s-054 0:54 "") # heretic E1M6s NoMo in 0:54 by Batawi
add_heretic_test(59/h1o6s057 0:57 "") # heretic E1M6s NoMo in 0:57 by JCD
add_heretic_test(481/h1p6s509 5:09 "") # heretic E1M6s Pacifist in 5:09 by Xit Vono
add_heretic_test(54170/heretice1m6s-10977 1:09 "") # heretic E1M6s SM Speed in 1:09 by Ks4
add_heretic_test(98/h1s6s113 1:13 "") # heretic E1M6s SM Speed in 1:13 by Vincent Catalaa
add_heretic_test(633/h1s6v221 2:20 "") # heretic E1M6s SM Speed in 2:20 by Vincent Catalaa
add_heretic_test(55876/h1f7-541 5:41 "") # heretic E1M7 BP Max in 5:41 by Archi
add_heretic_test(461/h1f7-833 8:33 "") # heretic E1M7 BP Max in 8:33 by Bruno Vergílio
add_heretic_test(75933/h1b7-111 1:11 "") # heretic E1M7 BP Speed in 1:11 by Insomnia
add_heretic_test(20/h1b7-117 1:17 "") # heretic E1M7 BP Speed in 1:17 by QWERTY
add_heretic_test(21/h1b7-131 1:31 "") # heretic E1M7 BP Speed in 1:31 by Vincent Catalaa
add_heretic_test(73293/h1x7-143 1:43 "") # heretic E1M7 NM 100S in 1:43 by Insomnia
add_heretic_test(51481/h1x7-145 1:45 "") # heretic E1M7 NM 100S in 1:45 by JCD
add_heretic_test(46/h1n7-120 1:20 "") # heretic E1M7 NM Speed in 1:20 by JCD
add_heretic_test(600/17nm 2:51 "-respawn") # heretic E1M7 NM Speed in 2:51 by Jeff N. Easthope
add_heretic_test(47538/h1o74583 0:45 "") # heretic E1M7 NoMo in 0:45 by depr4vity
add_heretic_test(60/h1o7-050 0:50 "") # heretic E1M7 NoMo in 0:50 by Kimo Xvirus
add_heretic_test(822/h1o7-053 0:53 "") # heretic E1M7 NoMo in 0:53 by Batawi
add_heretic_test(61/h1o75385 0:53 "") # heretic E1M7 NoMo in 0:53 by JCD
add_heretic_test(62/h1o75520 0:55 "-nomonsters") # heretic E1M7 NoMo in 0:55 by Vincent Catalaa
add_heretic_test(63/h1o75974 0:59 "-nomonsters") # heretic E1M7 NoMo in 0:59 by QWERTY
add_heretic_test(74187/o1s7-142 1:42 "") # heretic E1M7 NoMo 100S in 1:42 by El Tom
add_heretic_test(50728/o1s7-145 1:45 "") # heretic E1M7 NoMo 100S in 1:45 by PVS
add_heretic_test(72/h1p7-116 1:16 "") # heretic E1M7 Pacifist in 1:16 by JCD
add_heretic_test(72572/h1m7-542 5:42 "") # heretic E1M7 SM Max in 5:42 by Insomnia
add_heretic_test(55878/h1m7-602 6:02 "") # heretic E1M7 SM Max in 6:02 by Archi
add_heretic_test(470/h1m7-719 7:19 "") # heretic E1M7 SM Max in 7:19 by Vincent Catalaa
add_heretic_test(439/h1c7c035 0:35 "") # heretic E1M7 SM Speed in 0:35 by JCD, Ch0wW
add_heretic_test(681/h1c7trik 0:42 "") # heretic E1M7 SM Speed in 0:42 by Michael Lastovica, Vincent Catalaa
add_heretic_test(75585/h1s7-055 0:55 "") # heretic E1M7 SM Speed in 0:55 by Insomnia
add_heretic_test(65717/h1s7-056 0:56 "") # heretic E1M7 SM Speed in 0:56 by JCD
add_heretic_test(54172/heretice1m7-10003 1:00 "") # heretic E1M7 SM Speed in 1:00 by Ks4
add_heretic_test(99/h1s7-104 1:04 "") # heretic E1M7 SM Speed in 1:04 by QWERTY
add_heretic_test(100/h1s7-122 1:22 "") # heretic E1M7 SM Speed in 1:22 by Vincent Catalaa
add_heretic_test(634/h1s7v232 2:31 "") # heretic E1M7 SM Speed in 2:31 by Vincent Catalaa
add_heretic_test(75491/h1t7-1258 12:48 "") # heretic E1M7 Tyson in 12:48 by Insomnia
add_heretic_test(57004/heretice1m8bx-12914 1:29 "") # heretic E1M8 BP Max in 1:29 by Ks4
add_heretic_test(31/h1f8-238 2:38 "") # heretic E1M8 BP Max in 2:38 by Bruno Vergílio
add_heretic_test(54173/heretice1m8b-3011 0:30 "") # heretic E1M8 BP Speed in 0:30 by Ks4
add_heretic_test(22/h1b8-034 0:34 "") # heretic E1M8 BP Speed in 0:34 by QWERTY
add_heretic_test(23/h1b8-038 0:38 "") # heretic E1M8 BP Speed in 0:38 by QWERTY
add_heretic_test(24/h1b8-046 0:46 "") # heretic E1M8 BP Speed in 0:46 by Vincent Catalaa
add_heretic_test(73294/h1x8-044 0:44 "") # heretic E1M8 NM 100S in 0:44 by Insomnia
add_heretic_test(51482/h1x8-053 0:53 "") # heretic E1M8 NM 100S in 0:53 by JCD
add_heretic_test(47/h1n8-030 0:30 "") # heretic E1M8 NM Speed in 0:30 by JCD
add_heretic_test(48/h1n8-035 0:35 "") # heretic E1M8 NM Speed in 0:35 by JCD
add_heretic_test(600/18nm 1:35 "-respawn") # heretic E1M8 NM Speed in 1:35 by Jeff N. Easthope
add_heretic_test(73/h1p8-219 2:19 "") # heretic E1M8 Pacifist in 2:19 by JCD
add_heretic_test(74/h1p8-425 4:24 "") # heretic E1M8 Pacifist in 4:24 by Lucas Marincak
add_heretic_test(54091/h1m8c043 0:43 "") # heretic E1M8 SM Max in 0:43 by Ch0wW, JCD
add_heretic_test(72573/h1m8-124 1:24 "") # heretic E1M8 SM Max in 1:24 by Insomnia
add_heretic_test(56473/heretice1m8x-13757 1:37 "") # heretic E1M8 SM Max in 1:37 by Ks4
add_heretic_test(35/h1m8-213 2:13 "") # heretic E1M8 SM Max in 2:13 by Vincent Catalaa
add_heretic_test(440/h1c8c020 0:20 "") # heretic E1M8 SM Speed in 0:20 by JCD, Ch0wW
add_heretic_test(67399/h1s8-027 0:27 "") # heretic E1M8 SM Speed in 0:27 by JCD
add_heretic_test(101/h1s8-029 0:29 "") # heretic E1M8 SM Speed in 0:29 by Kimo Xvirus
add_heretic_test(102/h1s8-030 0:30 "") # heretic E1M8 SM Speed in 0:30 by Kimo Xvirus
add_heretic_test(103/h1s8-031 0:31 "") # heretic E1M8 SM Speed in 0:31 by Radek Pecka
add_heretic_test(104/h1s8-034 0:34 "") # heretic E1M8 SM Speed in 0:34 by QWERTY
add_heretic_test(105/h1s8-037 0:37 "") # heretic E1M8 SM Speed in 0:37 by Radek Pecka
add_heretic_test(106/h1s8-046 0:46 "") # heretic E1M8 SM Speed in 0:46 by Vincent Catalaa
add_heretic_test(621/h1s8v048 0:48 "") # heretic E1M8 SM Speed in 0:48 by Vincent Catalaa
add_heretic_test(112/h1t8-542 5:41 "") # heretic E1M8 Tyson in 5:41 by Lucas Marincak
add_heretic_test(77951/h1f9-402 4:02 "") # heretic E1M9 BP Max in 4:02 by Doomniel
add_heretic_test(462/h1f9-526 5:26 "") # heretic E1M9 BP Max in 5:26 by Vincent Catalaa
add_heretic_test(25/h1b9-114 1:14 "") # heretic E1M9 BP Speed in 1:14 by Vincent Catalaa
add_heretic_test(73295/h1x9-105 1:05 "") # heretic E1M9 NM 100S in 1:05 by Insomnia
add_heretic_test(51483/h1x9-110 1:10 "") # heretic E1M9 NM 100S in 1:10 by JCD
add_heretic_test(50/h1n9-116 1:16 "") # heretic E1M9 NM Speed in 1:16 by JCD
add_heretic_test(600/19nm 2:35 "-respawn") # heretic E1M9 NM Speed in 2:35 by Anonymous
add_heretic_test(76567/h1o9-057 0:57 "") # heretic E1M9 NoMo in 0:57 by Insomnia
add_heretic_test(47537/h1o9058 0:58 "") # heretic E1M9 NoMo in 0:58 by depr4vity
add_heretic_test(64/h1o95931 0:59 "-nomonsters") # heretic E1M9 NoMo in 0:59 by Vincent Catalaa
add_heretic_test(823/h1o9-059 0:59 "") # heretic E1M9 NoMo in 0:59 by Batawi
add_heretic_test(74188/o1s9-104 1:04 "") # heretic E1M9 NoMo 100S in 1:04 by El Tom
add_heretic_test(75/h1p9-058 0:58 "") # heretic E1M9 Pacifist in 0:58 by JCD
add_heretic_test(76/h1p9-059 0:59 "") # heretic E1M9 Pacifist in 0:59 by Vincent Catalaa
add_heretic_test(77/h1p9-101 1:01 "") # heretic E1M9 Pacifist in 1:01 by JCD
add_heretic_test(78/h1p9-102 1:02 "") # heretic E1M9 Pacifist in 1:02 by Vincent Catalaa
add_heretic_test(79/h1p9-103 1:03 "") # heretic E1M9 Pacifist in 1:03 by JCD
add_heretic_test(80/h1p9-121 1:20 "") # heretic E1M9 Pacifist in 1:20 by Lucas Marincak
add_heretic_test(72574/h1m9-413 4:13 "") # heretic E1M9 SM Max in 4:13 by Insomnia
add_heretic_test(471/h1m9-647 6:47 "") # heretic E1M9 SM Max in 6:47 by Vincent Catalaa
add_heretic_test(107/h1s9-105 1:05 "") # heretic E1M9 SM Speed in 1:05 by Vincent Catalaa
add_heretic_test(108/h1s9-111 1:11 "") # heretic E1M9 SM Speed in 1:11 by Vincent Catalaa
add_heretic_test(635/h1s9v146 1:45 "") # heretic E1M9 SM Speed in 1:45 by Vincent Catalaa
add_heretic_test(75492/h1t9-720 7:20 "") # heretic E1M9 Tyson in 7:20 by Insomnia
add_heretic_test(133/h2f1-106 1:06 "") # heretic E2M1 BP Max in 1:06 by Kimo Xvirus
add_heretic_test(134/h2f1-118 1:18 "") # heretic E2M1 BP Max in 1:18 by Xit Vono
add_heretic_test(135/h2f1-124 1:24 "") # heretic E2M1 BP Max in 1:24 by Vincent Catalaa
add_heretic_test(60100/heretic-e2m1b-2057 0:20 "") # heretic E2M1 BP Speed in 0:20 by Ks4
add_heretic_test(114/h2b1-021 0:21 "") # heretic E2M1 BP Speed in 0:21 by Vincent Catalaa
add_heretic_test(53453/h2x1-035 0:35 "") # heretic E2M1 NM 100S in 0:35 by JCD
add_heretic_test(75839/h2n1-2137 0:21 "") # heretic E2M1 NM Speed in 0:21 by El Tom
add_heretic_test(73568/h2n1-2323 0:23 "") # heretic E2M1 NM Speed in 0:23 by PrBoomerJuice
add_heretic_test(139/h2n1-024 0:24 "") # heretic E2M1 NM Speed in 0:24 by JCD
add_heretic_test(600/21nm 1:45 "-respawn") # heretic E2M1 NM Speed in 1:45 by Anonymous
add_heretic_test(47535/h2o11894 0:18 "") # heretic E2M1 NoMo in 0:18 by depr4vity
add_heretic_test(56828/h2o12097 0:20 "") # heretic E2M1 NoMo in 0:20 by Batawi
add_heretic_test(154/h2o12134 0:21 "") # heretic E2M1 NoMo in 0:21 by JCD
add_heretic_test(74189/o2s1-029 0:29 "") # heretic E2M1 NoMo 100S in 0:29 by El Tom
add_heretic_test(137/h2m1-145 1:45 "") # heretic E2M1 Other in 1:45 by Vincent Catalaa
add_heretic_test(47534/h2s1-019 0:19 "") # heretic E2M1 Pacifist in 0:19 by depr4vity
add_heretic_test(186/h2s1-020 0:20 "") # heretic E2M1 Pacifist in 0:20 by Kimo Xvirus
add_heretic_test(187/h2s1-022 0:22 "") # heretic E2M1 Pacifist in 0:22 by Radek Pecka
add_heretic_test(171/h2p1-024 0:24 "") # heretic E2M1 Pacifist in 0:24 by JCD
add_heretic_test(70909/h2m1-123 1:23 "") # heretic E2M1 SM Max in 1:23 by Insomnia
add_heretic_test(680/htuve2m1 5:14 "") # heretic E2M1 SM Max in 5:14 by Dave Turner
add_heretic_test(188/h2s1-024 0:24 "") # heretic E2M1 SM Speed in 0:24 by Vincent Catalaa
add_heretic_test(636/h2s1v033 0:32 "") # heretic E2M1 SM Speed in 0:32 by Vincent Catalaa
add_heretic_test(73885/h2o1sstr041 0:41 "") # heretic E2M1 Stroller in 0:41 by Insomnia
add_heretic_test(512/h2t1-302 3:02 "") # heretic E2M1 Tyson in 3:02 by Xit Vono
add_heretic_test(488/h2f2-325 3:25 "") # heretic E2M2 BP Max in 3:25 by Hitherto
add_heretic_test(489/h2f2-403 4:03 "") # heretic E2M2 BP Max in 4:03 by Xit Vono
add_heretic_test(490/h2f2-450 4:50 "") # heretic E2M2 BP Max in 4:50 by Vincent Catalaa
add_heretic_test(75936/h2b2-036 0:36 "") # heretic E2M2 BP Speed in 0:36 by Insomnia
add_heretic_test(54174/heretice2m2b-3829 0:38 "") # heretic E2M2 BP Speed in 0:38 by Ks4
add_heretic_test(115/h2b2-041 0:41 "") # heretic E2M2 BP Speed in 0:41 by Hitherto
add_heretic_test(116/h2b2-043 0:43 "") # heretic E2M2 BP Speed in 0:43 by Vincent Catalaa
add_heretic_test(117/h2b2-048 0:48 "") # heretic E2M2 BP Speed in 0:48 by QWERTY
add_heretic_test(118/h2b2-057 0:57 "") # heretic E2M2 BP Speed in 0:57 by Vincent Catalaa
add_heretic_test(51772/h2x2-110 1:10 "") # heretic E2M2 NM 100S in 1:10 by JCD
add_heretic_test(141/h2n2-042 0:42 "") # heretic E2M2 NM Speed in 0:42 by Hitherto
add_heretic_test(142/h2n2-046 0:46 "") # heretic E2M2 NM Speed in 0:46 by JCD
add_heretic_test(600/22nm 3:16 "-respawn") # heretic E2M2 NM Speed in 3:16 by Anonymous
add_heretic_test(47546/h2o23097 0:30 "") # heretic E2M2 NoMo in 0:30 by depr4vity
add_heretic_test(155/h2o23397 0:33 "-nomonsters") # heretic E2M2 NoMo in 0:33 by Hitherto
add_heretic_test(56829/h2o23411 0:34 "") # heretic E2M2 NoMo in 0:34 by Batawi
add_heretic_test(74190/o2s2-104 1:04 "") # heretic E2M2 NoMo 100S in 1:04 by El Tom
add_heretic_test(172/h2p2-059 0:59 "") # heretic E2M2 Pacifist in 0:59 by Hitherto
add_heretic_test(173/h2p2-116 1:16 "") # heretic E2M2 Pacifist in 1:16 by Xit Vono
add_heretic_test(75355/h2m2-323 3:23 "") # heretic E2M2 SM Max in 3:23 by Insomnia
add_heretic_test(497/h2m2-336 3:36 "") # heretic E2M2 SM Max in 3:36 by Hitherto
add_heretic_test(498/h2m2-447 4:47 "") # heretic E2M2 SM Max in 4:47 by Vincent Catalaa
add_heretic_test(47545/h2s2-034 0:34 "") # heretic E2M2 SM Speed in 0:34 by depr4vity
add_heretic_test(189/h2s2-038 0:38 "") # heretic E2M2 SM Speed in 0:38 by Hitherto
add_heretic_test(190/h2s2-043 0:43 "") # heretic E2M2 SM Speed in 0:43 by Radek Pecka
add_heretic_test(191/h2s2-044 0:44 "") # heretic E2M2 SM Speed in 0:44 by Vincent Catalaa
add_heretic_test(637/h2s2v055 0:54 "") # heretic E2M2 SM Speed in 0:54 by Vincent Catalaa
add_heretic_test(514/h2t21028 10:28 "") # heretic E2M2 Tyson in 10:28 by Xit Vono
add_heretic_test(513/h2t2-736 7:36 "") # heretic E2M2 Tyson in 7:36 by Hitherto
add_heretic_test(491/h2f3-517 5:17 "") # heretic E2M3 BP Max in 5:17 by Vincent Catalaa
add_heretic_test(119/h2b3-051 0:51 "") # heretic E2M3 BP Speed in 0:51 by QWERTY
add_heretic_test(120/h2b3-111 1:11 "") # heretic E2M3 BP Speed in 1:11 by Vincent Catalaa
add_heretic_test(143/h2n3-037 0:37 "") # heretic E2M3 NM Speed in 0:37 by JCD
add_heretic_test(144/h2n3-110 1:09 "-respawn") # heretic E2M3 NM Speed in 1:09 by Lucas Marincak
add_heretic_test(600/23nm 1:55 "-respawn") # heretic E2M3 NM Speed in 1:55 by Jeff N. Easthope
add_heretic_test(56830/h2o33463 0:34 "") # heretic E2M3 NoMo in 0:34 by Batawi
add_heretic_test(156/h2o33562 0:35 "") # heretic E2M3 NoMo in 0:35 by JCD
add_heretic_test(157/h2o3-056 0:56 "-nomonsters") # heretic E2M3 NoMo in 0:56 by Lucas Marincak
add_heretic_test(174/h2p3-111 1:11 "") # heretic E2M3 Pacifist in 1:11 by JCD
add_heretic_test(175/h2p3-128 1:28 "") # heretic E2M3 Pacifist in 1:28 by Xit Vono
add_heretic_test(75356/h2m3-412 4:12 "") # heretic E2M3 SM Max in 4:12 by Insomnia
add_heretic_test(499/h2m3-430 4:30 "") # heretic E2M3 SM Max in 4:30 by Vincent Catalaa
add_heretic_test(63414/h2s3-033 0:33 "") # heretic E2M3 SM Speed in 0:33 by JCD
add_heretic_test(441/h2c3j034 0:34 "") # heretic E2M3 SM Speed in 0:34 by JCD, Ch0wW
add_heretic_test(192/h2s3-037 0:37 "") # heretic E2M3 SM Speed in 0:37 by Vile
add_heretic_test(193/h2s3-041 0:41 "") # heretic E2M3 SM Speed in 0:41 by QWERTY
add_heretic_test(194/h2s3-102 1:02 "") # heretic E2M3 SM Speed in 1:02 by Vincent Catalaa
add_heretic_test(629/h2s3v211 2:10 "") # heretic E2M3 SM Speed in 2:10 by Vincent Catalaa
add_heretic_test(492/h2f4-830 8:30 "") # heretic E2M4 BP Max in 8:30 by Bruno Vergílio
add_heretic_test(121/h2b4-114 1:14 "") # heretic E2M4 BP Speed in 1:14 by QWERTY
add_heretic_test(51773/h2x4-118 1:18 "") # heretic E2M4 NM 100S in 1:18 by JCD
add_heretic_test(146/h2n4-111 1:11 "") # heretic E2M4 NM Speed in 1:11 by JCD
add_heretic_test(147/h2n4-125 1:25 "-respawn") # heretic E2M4 NM Speed in 1:25 by Lucas Marincak
add_heretic_test(600/24nm 2:26 "-respawn") # heretic E2M4 NM Speed in 2:26 by Anonymous
add_heretic_test(74080/h2o4-052 0:52 "") # heretic E2M4 NoMo in 0:52 by Insomnia
add_heretic_test(158/h2o45557 0:55 "") # heretic E2M4 NoMo in 0:55 by JCD
add_heretic_test(159/h2o4-057 0:57 "-nomonsters") # heretic E2M4 NoMo in 0:57 by Vile
add_heretic_test(57110/h2o4_5991 0:59 "") # heretic E2M4 NoMo in 0:59 by Batawi
add_heretic_test(160/h2o4-110 1:10 "-nomonsters") # heretic E2M4 NoMo in 1:10 by Lucas Marincak
add_heretic_test(74191/o2s4-112 1:12 "") # heretic E2M4 NoMo 100S in 1:12 by El Tom
add_heretic_test(67463/h2s4-047 0:47 "") # heretic E2M4 Pacifist in 0:47 by JCD
add_heretic_test(176/h2p4-114 1:14 "") # heretic E2M4 Pacifist in 1:14 by JCD
add_heretic_test(177/h2p4-128 1:28 "") # heretic E2M4 Pacifist in 1:28 by JCD
add_heretic_test(178/h2p4-149 1:49 "") # heretic E2M4 Pacifist in 1:49 by Xit Vono
add_heretic_test(75357/h2m4-555 5:51 "") # heretic E2M4 SM Max in 5:51 by Insomnia
add_heretic_test(500/h2m4-628 6:28 "") # heretic E2M4 SM Max in 6:28 by Vincent Catalaa
add_heretic_test(442/h2c4-028 0:28 "") # heretic E2M4 SM Speed in 0:28 by JCD, Suicider
add_heretic_test(60440/h2s4-049 0:49 "") # heretic E2M4 SM Speed in 0:49 by Mikolah
add_heretic_test(52435/h2s4-055 0:55 "") # heretic E2M4 SM Speed in 0:55 by Mikolah
add_heretic_test(195/h2s4-102 1:02 "") # heretic E2M4 SM Speed in 1:02 by JCD
add_heretic_test(196/h2s4-109 1:09 "") # heretic E2M4 SM Speed in 1:09 by QWERTY
add_heretic_test(197/h2s4-118 1:18 "") # heretic E2M4 SM Speed in 1:18 by Vincent Catalaa
add_heretic_test(638/h2s4v200 1:59 "") # heretic E2M4 SM Speed in 1:59 by Vincent Catalaa
add_heretic_test(122/h2b4s120 1:20 "") # heretic E2M4s BP Speed in 1:20 by QWERTY
add_heretic_test(148/h2n4s129 1:29 "") # heretic E2M4s NM Speed in 1:29 by Vincent Catalaa
add_heretic_test(74081/h2o4s-100 1:00 "") # heretic E2M4s NoMo in 1:00 by Insomnia
add_heretic_test(57109/h2o4s_10580 1:05 "") # heretic E2M4s NoMo in 1:05 by Batawi
add_heretic_test(161/h2o4s108 1:08 "") # heretic E2M4s NoMo in 1:08 by JCD
add_heretic_test(67464/h2s4s052 0:52 "") # heretic E2M4s Pacifist in 0:52 by JCD
add_heretic_test(179/h2p4s155 1:54 "") # heretic E2M4s Pacifist in 1:54 by Lucas Marincak
add_heretic_test(198/h2s4s115 1:15 "") # heretic E2M4s SM Speed in 1:15 by QWERTY
add_heretic_test(199/h2s4s132 1:32 "") # heretic E2M4s SM Speed in 1:32 by Vincent Catalaa
add_heretic_test(55854/h2f5-427 4:27 "") # heretic E2M5 BP Max in 4:27 by 4shockblast
add_heretic_test(493/h2f5-614 6:14 "") # heretic E2M5 BP Max in 6:14 by JCD
add_heretic_test(123/h2b5-022 0:22 "") # heretic E2M5 BP Speed in 0:22 by Vincent Catalaa
add_heretic_test(124/h2b5-025 0:25 "") # heretic E2M5 BP Speed in 0:25 by QWERTY
add_heretic_test(125/h2b5-026 0:26 "") # heretic E2M5 BP Speed in 0:26 by Opulent
add_heretic_test(51774/h2x5-124 1:24 "") # heretic E2M5 NM 100S in 1:24 by JCD
add_heretic_test(149/h2n5-020 0:20 "") # heretic E2M5 NM Speed in 0:20 by JCD
add_heretic_test(150/h2n5-022 0:22 "") # heretic E2M5 NM Speed in 0:22 by Vincent Catalaa
add_heretic_test(600/25nm2 0:55 "-respawn") # heretic E2M5 NM Speed in 0:55 by Anonymous
add_heretic_test(600/25nm 3:26 "-respawn") # heretic E2M5 NM Speed in 3:26 by Anonymous
add_heretic_test(164/h2o51794 0:17 "") # heretic E2M5 NoMo in 0:17 by Kimo Xvirus
add_heretic_test(57035/h2o5_1886 0:18 "") # heretic E2M5 NoMo in 0:18 by Batawi
add_heretic_test(162/h2o51985 0:19 "") # heretic E2M5 NoMo in 0:19 by JCD
add_heretic_test(163/h2o52097 0:20 "-nomonsters") # heretic E2M5 NoMo in 0:20 by QWERTY
add_heretic_test(74192/achu-109 1:09 "") # heretic E2M5 NoMo 100S in 1:09 by El Tom
add_heretic_test(50729/o2s5-119 1:19 "") # heretic E2M5 NoMo 100S in 1:19 by PVS
add_heretic_test(63413/h2p5-019 0:19 "") # heretic E2M5 Pacifist in 0:19 by JCD
add_heretic_test(201/h2s5-022 0:22 "") # heretic E2M5 Pacifist in 0:22 by QWERTY
add_heretic_test(180/h2p5-024 0:24 "") # heretic E2M5 Pacifist in 0:24 by JCD
add_heretic_test(75358/h2m5-438 4:38 "") # heretic E2M5 SM Max in 4:38 by Insomnia
add_heretic_test(501/h2m5-439 4:39 "") # heretic E2M5 SM Max in 4:39 by William Huber
add_heretic_test(200/h2s5-021 0:21 "") # heretic E2M5 SM Speed in 0:21 by Vincent Catalaa
add_heretic_test(202/h2s5-023 0:23 "") # heretic E2M5 SM Speed in 0:23 by Radek Pecka
add_heretic_test(203/h2s5-024 0:24 "") # heretic E2M5 SM Speed in 0:24 by Vincent Catalaa
add_heretic_test(639/h2s5v136 1:35 "") # heretic E2M5 SM Speed in 1:35 by Vincent Catalaa
add_heretic_test(494/h2f6-655 6:55 "") # heretic E2M6 BP Max in 6:55 by Bruno Vergílio
add_heretic_test(54176/heretice2m6b-10680 1:06 "") # heretic E2M6 BP Speed in 1:06 by Ks4
add_heretic_test(126/h2b6-124 1:24 "") # heretic E2M6 BP Speed in 1:24 by QWERTY
add_heretic_test(127/h2b6-153 1:53 "") # heretic E2M6 BP Speed in 1:53 by Vincent Catalaa
add_heretic_test(51775/h2x6-134 1:34 "") # heretic E2M6 NM 100S in 1:34 by JCD
add_heretic_test(600/26nm 2:35 "-respawn") # heretic E2M6 NM Speed in 2:35 by Jeff N. Easthope
add_heretic_test(72099/h2o6s054 0:54 "") # heretic E2M6 NoMo in 0:54 by Insomnia
add_heretic_test(57036/h2o6_5723 0:57 "") # heretic E2M6 NoMo in 0:57 by Batawi
add_heretic_test(165/h2o65911 0:59 "") # heretic E2M6 NoMo in 0:59 by JCD
add_heretic_test(166/h2o6-108 1:08 "-nomonsters") # heretic E2M6 NoMo in 1:08 by Lucas Marincak
add_heretic_test(74193/o2s6-121 1:21 "") # heretic E2M6 NoMo 100S in 1:21 by El Tom
add_heretic_test(50730/o2s6-127 1:27 "") # heretic E2M6 NoMo 100S in 1:27 by PVS
add_heretic_test(181/h2p6-205 2:05 "") # heretic E2M6 Pacifist in 2:05 by Xit Vono
add_heretic_test(75359/h2m6-454 4:54 "") # heretic E2M6 SM Max in 4:54 by Insomnia
add_heretic_test(502/h2m6-558 5:58 "") # heretic E2M6 SM Max in 5:58 by Vincent Catalaa
add_heretic_test(54175/heretice2m6-10571 1:05 "") # heretic E2M6 SM Speed in 1:05 by Ks4
add_heretic_test(204/h2s6-117 1:17 "") # heretic E2M6 SM Speed in 1:17 by Vincent Catalaa
add_heretic_test(640/h2s6v143 1:42 "") # heretic E2M6 SM Speed in 1:42 by Vincent Catalaa
add_heretic_test(495/h2f7-606 6:06 "") # heretic E2M7 BP Max in 6:06 by Vincent Catalaa
add_heretic_test(128/h2b7-104 1:04 "") # heretic E2M7 BP Speed in 1:04 by QWERTY
add_heretic_test(129/h2b7-129 1:29 "") # heretic E2M7 BP Speed in 1:29 by Vincent Catalaa
add_heretic_test(51776/h2x7-125 1:25 "") # heretic E2M7 NM 100S in 1:25 by JCD
add_heretic_test(600/27nm 4:00 "-respawn") # heretic E2M7 NM Speed in 4:00 by Anonymous
add_heretic_test(73597/h2o7-052 0:52 "") # heretic E2M7 NoMo in 0:52 by Insomnia
add_heretic_test(57037/h2o7_5469 0:54 "") # heretic E2M7 NoMo in 0:54 by Batawi
add_heretic_test(167/h2o75651 0:56 "") # heretic E2M7 NoMo in 0:56 by JCD
add_heretic_test(168/h2o7-111 1:11 "-nomonsters") # heretic E2M7 NoMo in 1:11 by Lucas Marincak
add_heretic_test(74194/o2s7-117 1:17 "") # heretic E2M7 NoMo 100S in 1:17 by El Tom
add_heretic_test(50731/o2s7-126 1:26 "") # heretic E2M7 NoMo 100S in 1:26 by PVS
add_heretic_test(182/h2p7-202 2:02 "") # heretic E2M7 Pacifist in 2:02 by Xit Vono
add_heretic_test(75360/h2m7-422 4:22 "") # heretic E2M7 SM Max in 4:22 by Insomnia
add_heretic_test(503/h2m7-518 5:18 "") # heretic E2M7 SM Max in 5:18 by Vincent Catalaa
add_heretic_test(69120/h2-7-5986 0:59 "") # heretic E2M7 SM Speed in 0:59 by Ks4
add_heretic_test(205/h2s7-103 1:03 "") # heretic E2M7 SM Speed in 1:03 by QWERTY
add_heretic_test(206/h2s7-110 1:10 "") # heretic E2M7 SM Speed in 1:10 by Radek Pecka
add_heretic_test(207/h2s7-123 1:23 "") # heretic E2M7 SM Speed in 1:23 by Vincent Catalaa
add_heretic_test(641/h2s7v142 1:41 "") # heretic E2M7 SM Speed in 1:41 by Vincent Catalaa
add_heretic_test(136/h2f8-227 2:27 "") # heretic E2M8 BP Max in 2:27 by Vincent Catalaa
add_heretic_test(54178/heretice2m8b-10603 1:06 "") # heretic E2M8 BP Speed in 1:06 by Ks4
add_heretic_test(130/h2b8-218 2:18 "") # heretic E2M8 BP Speed in 2:18 by Vincent Catalaa
add_heretic_test(487/h2b8-855 8:55 "") # heretic E2M8 BP Speed in 8:55 by Anonymous
add_heretic_test(53454/h2x8-216 2:16 "") # heretic E2M8 NM 100S in 2:16 by JCD
add_heretic_test(152/h2n8-321 3:21 "") # heretic E2M8 NM Speed in 3:21 by Vincent Catalaa
add_heretic_test(511/h2p81120 11:20 "") # heretic E2M8 Pacifist in 11:20 by PVS
add_heretic_test(183/h2p8-257 2:57 "") # heretic E2M8 Pacifist in 2:57 by JCD
add_heretic_test(54092/h2m8c058 0:58 "") # heretic E2M8 SM Max in 0:58 by Ch0wW, JCD
add_heretic_test(75361/h2m8-118 1:18 "") # heretic E2M8 SM Max in 1:18 by Insomnia
add_heretic_test(54179/heretice2m8x-12017 1:20 "") # heretic E2M8 SM Max in 1:20 by Ks4
add_heretic_test(138/h2m8-153 1:53 "") # heretic E2M8 SM Max in 1:53 by Vincent Catalaa
add_heretic_test(54177/heretice2m8-59,71 0:59 "") # heretic E2M8 SM Speed in 0:59 by Ks4
add_heretic_test(208/h2s8-103 1:03 "") # heretic E2M8 SM Speed in 1:03 by Vincent Catalaa
add_heretic_test(642/h2s8v409 4:08 "") # heretic E2M8 SM Speed in 4:08 by Vincent Catalaa
add_heretic_test(515/h2t8-922 9:22 "") # heretic E2M8 Tyson in 9:22 by Lucas Marincak
add_heretic_test(77334/h2f9-433 4:33 "") # heretic E2M9 BP Max in 4:33 by Doomniel
add_heretic_test(496/h2f9-743 7:43 "") # heretic E2M9 BP Max in 7:43 by Vincent Catalaa
add_heretic_test(131/h2b9-051 0:51 "") # heretic E2M9 BP Speed in 0:51 by QWERTY
add_heretic_test(132/h2b9-101 1:01 "") # heretic E2M9 BP Speed in 1:01 by Vincent Catalaa
add_heretic_test(51777/h2x9-204 2:04 "") # heretic E2M9 NM 100S in 2:04 by JCD
add_heretic_test(153/h2n9-058 0:57 "-respawn") # heretic E2M9 NM Speed in 0:57 by Lucas Marincak
add_heretic_test(600/29nm 2:34 "-respawn") # heretic E2M9 NM Speed in 2:34 by Jeff N. Easthope
add_heretic_test(76282/h2o93757 0:37 "") # heretic E2M9 NoMo in 0:37 by Ks4
add_heretic_test(73271/h2o93843 0:38 "") # heretic E2M9 NoMo in 0:38 by PrBoomerJuice
add_heretic_test(169/h2o93879 0:38 "") # heretic E2M9 NoMo in 0:38 by JCD
add_heretic_test(57111/h2o9_3949 0:39 "") # heretic E2M9 NoMo in 0:39 by Batawi
add_heretic_test(170/h2o9-050 0:50 "-nomonsters") # heretic E2M9 NoMo in 0:50 by Lucas Marincak
add_heretic_test(74195/o2s9-150 1:50 "") # heretic E2M9 NoMo 100S in 1:50 by El Tom
add_heretic_test(50732/o2s9-157 1:57 "") # heretic E2M9 NoMo 100S in 1:57 by PVS
add_heretic_test(184/h2p9-054 0:54 "") # heretic E2M9 Pacifist in 0:54 by JCD
add_heretic_test(185/h2p9-058 0:58 "") # heretic E2M9 Pacifist in 0:58 by Lucas Marincak
add_heretic_test(75362/h2m9-437 4:37 "") # heretic E2M9 SM Max in 4:37 by Insomnia
add_heretic_test(504/h2m9-555 5:55 "") # heretic E2M9 SM Max in 5:55 by Vincent Catalaa
add_heretic_test(54081/heretice2m9-4891 0:48 "") # heretic E2M9 SM Speed in 0:48 by Ks4
add_heretic_test(209/h2s9-054 0:54 "") # heretic E2M9 SM Speed in 0:54 by Vincent Catalaa
add_heretic_test(643/h2s9v149 1:42 "") # heretic E2M9 SM Speed in 1:42 by Vincent Catalaa
add_heretic_test(517/h3f1-431 4:31 "") # heretic E3M1 BP Max in 4:31 by Xit Vono
add_heretic_test(518/h3f1-449 4:49 "") # heretic E3M1 BP Max in 4:49 by Vincent Catalaa
add_heretic_test(57006/heretice3m1b-2334 0:23 "") # heretic E3M1 BP Speed in 0:23 by Ks4
add_heretic_test(210/h3b1-025 0:25 "") # heretic E3M1 BP Speed in 0:25 by Xit Vono
add_heretic_test(211/h3b1-026 0:26 "") # heretic E3M1 BP Speed in 0:26 by QWERTY
add_heretic_test(212/h3b1-040 0:40 "") # heretic E3M1 BP Speed in 0:40 by Radek Pecka
add_heretic_test(51778/h3x1-031 0:31 "") # heretic E3M1 NM 100S in 0:31 by JCD
add_heretic_test(57073/h3p1-26 0:26 "") # heretic E3M1 NM Speed in 0:26 by kvothesixstring
add_heretic_test(229/h3n1-032 0:32 "") # heretic E3M1 NM Speed in 0:32 by JCD
add_heretic_test(230/h3n1-038 0:38 "") # heretic E3M1 NM Speed in 0:38 by Vincent Catalaa
add_heretic_test(600/31nm 1:37 "-respawn") # heretic E3M1 NM Speed in 1:37 by Anonymous
add_heretic_test(243/h3o14271 0:42 "") # heretic E3M1 NoMo in 0:42 by Kimo Xvirus
add_heretic_test(57112/h3o1_4494 0:44 "") # heretic E3M1 NoMo in 0:44 by Batawi
add_heretic_test(242/h3o14985 0:49 "") # heretic E3M1 NoMo in 0:49 by JCD
add_heretic_test(74196/o3s1-057 0:57 "") # heretic E3M1 NoMo 100S in 0:57 by El Tom
add_heretic_test(57279/h3p1-023 0:23 "") # heretic E3M1 Pacifist in 0:23 by Kyle McAwesome
add_heretic_test(253/h3p1-028 0:28 "") # heretic E3M1 Pacifist in 0:28 by JCD
add_heretic_test(254/h3p1-030 0:30 "") # heretic E3M1 Pacifist in 0:30 by Xit Vono
add_heretic_test(255/h3p1-033 0:33 "") # heretic E3M1 Pacifist in 0:33 by JCD
add_heretic_test(76683/h3m1-422 4:22 "") # heretic E3M1 SM Max in 4:22 by Insomnia
add_heretic_test(528/h3m1-439 4:39 "") # heretic E3M1 SM Max in 4:39 by Vincent Catalaa
add_heretic_test(57005/heretice3m1-2309 0:23 "") # heretic E3M1 SM Speed in 0:23 by Ks4
add_heretic_test(54391/e3m1-02614 0:26 "") # heretic E3M1 SM Speed in 0:26 by PBeGood4
add_heretic_test(265/h3s1-026 0:26 "") # heretic E3M1 SM Speed in 0:26 by Xit Vono
add_heretic_test(266/h3s1-028 0:28 "") # heretic E3M1 SM Speed in 0:28 by Radek Pecka
add_heretic_test(267/h3s1-032 0:32 "") # heretic E3M1 SM Speed in 0:32 by Vincent Catalaa
add_heretic_test(540/h3t11017 10:17 "") # heretic E3M1 Tyson in 10:17 by Xit Vono
add_heretic_test(519/h3f2-456 4:56 "") # heretic E3M2 BP Max in 4:56 by Vincent Catalaa
add_heretic_test(213/h3b2-038 0:38 "") # heretic E3M2 BP Speed in 0:38 by QWERTY
add_heretic_test(214/h3b2-059 0:59 "") # heretic E3M2 BP Speed in 0:59 by Vincent Catalaa
add_heretic_test(51779/h3x2-054 0:54 "") # heretic E3M2 NM 100S in 0:54 by JCD
add_heretic_test(232/h3n2-036 0:36 "") # heretic E3M2 NM Speed in 0:36 by Vincent Catalaa
add_heretic_test(600/32nm 1:13 "-respawn") # heretic E3M2 NM Speed in 1:13 by Jeff N. Easthope
add_heretic_test(76284/h3o23054 0:30 "") # heretic E3M2 NoMo in 0:30 by Ks4
add_heretic_test(245/h3o23194 0:31 "") # heretic E3M2 NoMo in 0:31 by Kimo Xvirus
add_heretic_test(57113/h3o2_3434 0:34 "") # heretic E3M2 NoMo in 0:34 by Batawi
add_heretic_test(244/h3o24111 0:41 "") # heretic E3M2 NoMo in 0:41 by JCD
add_heretic_test(74197/o3s2-043 0:43 "") # heretic E3M2 NoMo 100S in 0:43 by El Tom
add_heretic_test(256/h3p2-051 0:51 "") # heretic E3M2 Pacifist in 0:51 by Lucas Marincak
add_heretic_test(76684/h3m2-409 4:09 "") # heretic E3M2 SM Max in 4:09 by Insomnia
add_heretic_test(529/h3m2-708 7:08 "") # heretic E3M2 SM Max in 7:08 by Bruno Vergílio
add_heretic_test(268/h3s2-031 0:31 "") # heretic E3M2 SM Speed in 0:31 by JCD
add_heretic_test(269/h3s2-038 0:38 "") # heretic E3M2 SM Speed in 0:38 by QWERTY
add_heretic_test(270/h3s2-040 0:40 "") # heretic E3M2 SM Speed in 0:40 by Vincent Catalaa
add_heretic_test(271/h3s2-041 0:41 "") # heretic E3M2 SM Speed in 0:41 by QWERTY
add_heretic_test(272/h3s2-046 0:46 "") # heretic E3M2 SM Speed in 0:46 by Vincent Catalaa
add_heretic_test(622/h3s2-059 0:59 "") # heretic E3M2 SM Speed in 0:59 by Vincent Catalaa
add_heretic_test(520/h3f3-731 7:31 "") # heretic E3M3 BP Max in 7:31 by Bruno Vergílio
add_heretic_test(58045/h3b3-019 0:19 "") # heretic E3M3 BP Speed in 0:19 by Mikolah
add_heretic_test(57008/heretice3m3b-10534 1:05 "") # heretic E3M3 BP Speed in 1:05 by Ks4
add_heretic_test(54082/heretice3m3b-11566 1:15 "") # heretic E3M3 BP Speed in 1:15 by Ks4
add_heretic_test(215/h3b3-123 1:23 "") # heretic E3M3 BP Speed in 1:23 by QWERTY
add_heretic_test(216/h3b3-146 1:46 "") # heretic E3M3 BP Speed in 1:46 by Vincent Catalaa
add_heretic_test(51780/h3x3-135 1:35 "") # heretic E3M3 NM 100S in 1:35 by JCD
add_heretic_test(600/33nm 2:52 "-respawn") # heretic E3M3 NM Speed in 2:52 by Anonymous
add_heretic_test(76569/h3o35126 0:51 "") # heretic E3M3 NoMo in 0:51 by Ks4
add_heretic_test(57114/h3o3_5697 0:56 "") # heretic E3M3 NoMo in 0:56 by Batawi
add_heretic_test(246/h3o3-057 0:57 "") # heretic E3M3 NoMo in 0:57 by JCD
add_heretic_test(74198/o3s3-121 1:21 "") # heretic E3M3 NoMo 100S in 1:21 by El Tom
add_heretic_test(257/h3p3-115 1:15 "") # heretic E3M3 Pacifist in 1:15 by JCD
add_heretic_test(258/h3p3-205 2:05 "") # heretic E3M3 Pacifist in 2:05 by Lucas Marincak
add_heretic_test(76685/h3m3-538 5:38 "") # heretic E3M3 SM Max in 5:38 by Insomnia
add_heretic_test(530/h3m3-844 8:44 "") # heretic E3M3 SM Max in 8:44 by Bruno Vergílio
add_heretic_test(57007/heretice3m3-10477 1:04 "") # heretic E3M3 SM Speed in 1:04 by Ks4
add_heretic_test(273/h3s3-113 1:13 "") # heretic E3M3 SM Speed in 1:13 by Vincent Catalaa
add_heretic_test(623/h3s3-130 1:30 "") # heretic E3M3 SM Speed in 1:30 by Vincent Catalaa
add_heretic_test(521/h3f4-812 8:12 "") # heretic E3M4 BP Max in 8:12 by Bruno Vergílio
add_heretic_test(54084/heretice3m4b-11523 1:15 "") # heretic E3M4 BP Speed in 1:15 by Ks4
add_heretic_test(217/h3b4-121 1:21 "") # heretic E3M4 BP Speed in 1:21 by QWERTY
add_heretic_test(218/h3b4-140 1:40 "") # heretic E3M4 BP Speed in 1:40 by Vincent Catalaa
add_heretic_test(51781/h3x4-152 1:52 "") # heretic E3M4 NM 100S in 1:52 by JCD
add_heretic_test(234/h3n4-111 1:11 "") # heretic E3M4 NM Speed in 1:11 by Vincent Catalaa
add_heretic_test(76570/h3o410229 1:02 "") # heretic E3M4 NoMo in 1:02 by Ks4
add_heretic_test(57195/h3o4_10391 1:03 "") # heretic E3M4 NoMo in 1:03 by Batawi
add_heretic_test(247/h3o4-104 1:04 "") # heretic E3M4 NoMo in 1:04 by JCD
add_heretic_test(685/h3o4-105 1:05 "-nomonsters") # heretic E3M4 NoMo in 1:05 by PVS
add_heretic_test(74199/o3s4-121 1:42 "") # heretic E3M4 NoMo 100S in 1:42 by El Tom
add_heretic_test(259/h3p4-207 2:07 "") # heretic E3M4 Pacifist in 2:07 by Vincent Catalaa
add_heretic_test(531/h3m41041 10:41 "") # heretic E3M4 SM Max in 10:41 by Bruno Vergílio
add_heretic_test(76686/h3m4-612 6:12 "") # heretic E3M4 SM Max in 6:12 by Insomnia
add_heretic_test(69121/h3-4-11040 1:10 "") # heretic E3M4 SM Speed in 1:10 by Ks4
add_heretic_test(54083/heretice3m4-11520 1:15 "") # heretic E3M4 SM Speed in 1:15 by Ks4
add_heretic_test(274/h3s4-119 1:19 "") # heretic E3M4 SM Speed in 1:19 by Vincent Catalaa
add_heretic_test(219/h3b4s112 1:12 "") # heretic E3M4s BP Speed in 1:12 by Vincent Catalaa
add_heretic_test(220/h3b4s120 1:20 "") # heretic E3M4s BP Speed in 1:20 by Vincent Catalaa
add_heretic_test(55715/h3n4s103 1:03 "") # heretic E3M4s NM Speed in 1:03 by 4shockblast
add_heretic_test(236/h3n4s105 1:05 "") # heretic E3M4s NM Speed in 1:05 by Vincent Catalaa
add_heretic_test(600/34nm 2:03 "-respawn") # heretic E3M4s NM Speed in 2:03 by Anonymous
add_heretic_test(64958/h3o4s055 0:55 "") # heretic E3M4s NoMo in 0:55 by JCD
add_heretic_test(63416/h3s4so5800 0:58 "") # heretic E3M4s NoMo in 0:58 by AmnesiaSilence
add_heretic_test(57194/h3o4s_5840 0:58 "") # heretic E3M4s NoMo in 0:58 by Batawi
add_heretic_test(248/h3o4s059 0:59 "") # heretic E3M4s NoMo in 0:59 by JCD
add_heretic_test(686/h3o4s059 0:59 "-nomonsters") # heretic E3M4s NoMo in 0:59 by PVS
add_heretic_test(260/h3p4s232 2:32 "") # heretic E3M4s Pacifist in 2:32 by Lucas Marincak
add_heretic_test(69122/h3-4s-10360 1:03 "") # heretic E3M4s SM Speed in 1:03 by Ks4
add_heretic_test(63415/h3s4s-11237 1:12 "") # heretic E3M4s SM Speed in 1:12 by AmnesiaSilence
add_heretic_test(275/h3s4s114 1:14 "") # heretic E3M4s SM Speed in 1:14 by Vincent Catalaa
add_heretic_test(522/h3f5-708 7:08 "") # heretic E3M5 BP Max in 7:08 by Bruno Vergílio
add_heretic_test(64955/h3-5b-4617 0:46 "") # heretic E3M5 BP Speed in 0:46 by Ks4
add_heretic_test(54086/heretice3m5b-5054 0:50 "") # heretic E3M5 BP Speed in 0:50 by Ks4
add_heretic_test(221/h3b5-052 0:52 "") # heretic E3M5 BP Speed in 0:52 by QWERTY
add_heretic_test(222/h3b5-058 0:58 "") # heretic E3M5 BP Speed in 0:58 by Vincent Catalaa
add_heretic_test(51782/h3x5-121 1:21 "") # heretic E3M5 NM 100S in 1:21 by JCD
add_heretic_test(600/35nm2 2:01 "-respawn") # heretic E3M5 NM Speed in 2:01 by Anonymous
add_heretic_test(76285/h3o54066 0:40 "") # heretic E3M5 NoMo in 0:40 by Ks4
add_heretic_test(57196/h3o5_4277 0:42 "") # heretic E3M5 NoMo in 0:42 by Batawi
add_heretic_test(249/h3o54485 0:44 "") # heretic E3M5 NoMo in 0:44 by JCD
add_heretic_test(74200/o3s5-107 1:07 "") # heretic E3M5 NoMo 100S in 1:07 by El Tom
add_heretic_test(55716/h3p5-055 0:55 "") # heretic E3M5 Pacifist in 0:55 by 4shockblast
add_heretic_test(261/h3p5-106 1:06 "") # heretic E3M5 Pacifist in 1:06 by Vincent Catalaa
add_heretic_test(76687/h3m5-421 4:21 "") # heretic E3M5 SM Max in 4:21 by Insomnia
add_heretic_test(532/h3m5-730 7:30 "") # heretic E3M5 SM Max in 7:30 by Bruno Vergílio
add_heretic_test(64954/h3-5-4617 0:46 "") # heretic E3M5 SM Speed in 0:46 by Ks4
add_heretic_test(54085/heretice3m5-4977 0:49 "") # heretic E3M5 SM Speed in 0:49 by Ks4
add_heretic_test(276/h3s5-055 0:55 "") # heretic E3M5 SM Speed in 0:55 by QWERTY
add_heretic_test(277/h3s5-057 0:57 "") # heretic E3M5 SM Speed in 0:57 by Vincent Catalaa
add_heretic_test(76568/h3f6-431 4:31 "") # heretic E3M6 BP Max in 4:31 by Doomniel
add_heretic_test(76283/h3f6-458 4:58 "") # heretic E3M6 BP Max in 4:58 by Doomniel
add_heretic_test(523/h3f6-647 6:47 "") # heretic E3M6 BP Max in 6:47 by Bruno Vergílio
add_heretic_test(223/h3b6-113 1:13 "") # heretic E3M6 BP Speed in 1:13 by Vincent Catalaa
add_heretic_test(51783/h3x6-158 1:58 "") # heretic E3M6 NM 100S in 1:58 by JCD
add_heretic_test(600/36nm 1:55 "-respawn") # heretic E3M6 NM Speed in 1:55 by Anonymous
add_heretic_test(76286/h3o64360 0:43 "") # heretic E3M6 NoMo in 0:43 by Ks4
add_heretic_test(57197/h3o6_4497 0:44 "") # heretic E3M6 NoMo in 0:44 by Batawi
add_heretic_test(250/h3o65711 0:57 "") # heretic E3M6 NoMo in 0:57 by JCD
add_heretic_test(74201/o3s6-121 1:21 "") # heretic E3M6 NoMo 100S in 1:21 by El Tom
add_heretic_test(262/h3p6-106 1:06 "") # heretic E3M6 Pacifist in 1:06 by Vincent Catalaa
add_heretic_test(76688/h3m6-428 4:28 "") # heretic E3M6 SM Max in 4:28 by Insomnia
add_heretic_test(533/h3m6-712 7:12 "") # heretic E3M6 SM Max in 7:12 by Bruno Vergílio
add_heretic_test(443/h3c6c016 0:16 "") # heretic E3M6 SM Speed in 0:16 by JCD, Ch0wW
add_heretic_test(69123/h3-6-5446 0:54 "") # heretic E3M6 SM Speed in 0:54 by Ks4
add_heretic_test(278/h3s6-058 0:58 "") # heretic E3M6 SM Speed in 0:58 by Vincent Catalaa
add_heretic_test(279/h3s6-107 1:07 "") # heretic E3M6 SM Speed in 1:07 by Vincent Catalaa
add_heretic_test(280/h3s6-117 1:17 "") # heretic E3M6 SM Speed in 1:17 by Vincent Catalaa
add_heretic_test(524/h3f71257 12:57 "") # heretic E3M7 BP Max in 12:57 by Bruno Vergílio
add_heretic_test(224/h3b7-223 2:23 "") # heretic E3M7 BP Speed in 2:23 by Vincent Catalaa
add_heretic_test(51784/h3x7-218 2:18 "") # heretic E3M7 NM 100S in 2:18 by JCD
add_heretic_test(600/37nm 3:19 "-respawn") # heretic E3M7 NM Speed in 3:19 by Jeff N. Easthope
add_heretic_test(57198/h3o7_11366 1:13 "") # heretic E3M7 NoMo in 1:13 by Batawi
add_heretic_test(251/h3o7-119 1:19 "") # heretic E3M7 NoMo in 1:19 by JCD
add_heretic_test(74202/o3s7-154 1:54 "") # heretic E3M7 NoMo 100S in 1:54 by El Tom
add_heretic_test(263/h3p7-213 2:13 "") # heretic E3M7 Pacifist in 2:13 by Vincent Catalaa
add_heretic_test(76689/h3m7-605 6:05 "") # heretic E3M7 SM Max in 6:05 by Insomnia
add_heretic_test(534/h3m7-942 9:42 "") # heretic E3M7 SM Max in 9:42 by Bruno Vergílio
add_heretic_test(73598/h3s7-12923 1:29 "") # heretic E3M7 SM Speed in 1:29 by Ks4
add_heretic_test(54087/heretice3m7-13534 1:35 "") # heretic E3M7 SM Speed in 1:35 by Ks4
add_heretic_test(281/h3s7-139 1:39 "") # heretic E3M7 SM Speed in 1:39 by Vincent Catalaa
add_heretic_test(282/h3s7-141 1:41 "") # heretic E3M7 SM Speed in 1:41 by Vincent Catalaa
add_heretic_test(283/h3s7-158 1:58 "") # heretic E3M7 SM Speed in 1:58 by Vincent Catalaa
add_heretic_test(525/h3f8-325 3:25 "") # heretic E3M8 BP Max in 3:25 by Xit Vono
add_heretic_test(526/h3f8-443 4:43 "") # heretic E3M8 BP Max in 4:43 by Vincent Catalaa
add_heretic_test(69124/h3-8b-5563 0:55 "") # heretic E3M8 BP Speed in 0:55 by Ks4
add_heretic_test(239/h3n8-217 2:17 "") # heretic E3M8 NM Speed in 2:17 by JCD
add_heretic_test(538/h3n8-415 4:15 "") # heretic E3M8 NM Speed in 4:15 by Xit Vono
add_heretic_test(539/h3n8-445 4:45 "") # heretic E3M8 NM Speed in 4:45 by Vile
add_heretic_test(54093/h3m8c051 0:51 "") # heretic E3M8 SM Max in 0:51 by Ch0wW, JCD
add_heretic_test(64957/h3m8-106 1:06 "") # heretic E3M8 SM Max in 1:06 by JCD
add_heretic_test(227/h3m8-114 1:14 "") # heretic E3M8 SM Max in 1:14 by Xit Vono
add_heretic_test(228/h3m8-228 2:28 "") # heretic E3M8 SM Max in 2:28 by Vincent Catalaa
add_heretic_test(65749/h3s8x045 0:45 "") # heretic E3M8 SM Speed in 0:45 by JCD
add_heretic_test(71879/h3s8-046 0:46 "") # heretic E3M8 SM Speed in 0:46 by El Juancho
add_heretic_test(71334/h3s8-047 0:47 "") # heretic E3M8 SM Speed in 0:47 by JCD
add_heretic_test(284/h3s8-054 0:54 "") # heretic E3M8 SM Speed in 0:54 by Kimo Xvirus
add_heretic_test(444/h3c8j054 0:54 "") # heretic E3M8 SM Speed in 0:54 by JCD, Ch0wW
add_heretic_test(285/h3s8-112 1:12 "") # heretic E3M8 SM Speed in 1:12 by JCD
add_heretic_test(286/h3s8-113 1:13 "") # heretic E3M8 SM Speed in 1:13 by Xit Vono
add_heretic_test(287/h3s8-120 1:20 "") # heretic E3M8 SM Speed in 1:20 by Vincent Catalaa
add_heretic_test(624/h3s8v330 3:30 "") # heretic E3M8 SM Speed in 3:30 by Vincent Catalaa
add_heretic_test(687/h3m8-635 6:35 "") # heretic E3M8 SM Speed in 6:35 by Steve Dudzik
add_heretic_test(541/h3t8-355 3:55 "") # heretic E3M8 Tyson in 3:55 by Xit Vono
add_heretic_test(64956/h3f9-319 3:19 "") # heretic E3M9 BP Max in 3:19 by JCD
add_heretic_test(527/h3f9-543 5:43 "") # heretic E3M9 BP Max in 5:43 by Vincent Catalaa
add_heretic_test(56475/heretice3m9b-4154 0:41 "") # heretic E3M9 BP Speed in 0:41 by Ks4
add_heretic_test(225/h3b9-045 0:45 "") # heretic E3M9 BP Speed in 0:45 by QWERTY
add_heretic_test(226/h3b9-047 0:47 "") # heretic E3M9 BP Speed in 0:47 by Vincent Catalaa
add_heretic_test(51785/h3x9-126 1:26 "") # heretic E3M9 NM 100S in 1:26 by JCD
add_heretic_test(73569/h3n9-3966 0:39 "") # heretic E3M9 NM Speed in 0:39 by PrBoomerJuice
add_heretic_test(240/h3n9-043 0:43 "") # heretic E3M9 NM Speed in 0:43 by JCD
add_heretic_test(689/h3n9-058 0:58 "") # heretic E3M9 NM Speed in 0:58 by JCD
add_heretic_test(600/39nm 1:17 "-respawn") # heretic E3M9 NM Speed in 1:17 by Anonymous
add_heretic_test(57199/h3o9_3197 0:31 "") # heretic E3M9 NoMo in 0:31 by Batawi
add_heretic_test(252/h3o93274 0:32 "") # heretic E3M9 NoMo in 0:32 by JCD
add_heretic_test(74203/o3s9-124 1:24 "") # heretic E3M9 NoMo 100S in 1:24 by El Tom
add_heretic_test(264/h3p9-040 0:40 "") # heretic E3M9 Pacifist in 0:40 by Vincent Catalaa
add_heretic_test(76690/h3m9-403 4:03 "") # heretic E3M9 SM Max in 4:03 by Insomnia
add_heretic_test(535/h3m9-547 5:47 "") # heretic E3M9 SM Max in 5:47 by Vincent Catalaa
add_heretic_test(63417/h3s9-030 0:30 "") # heretic E3M9 SM Speed in 0:30 by JCD
add_heretic_test(56474/heretice3m9-3340 0:33 "") # heretic E3M9 SM Speed in 0:33 by Ks4
add_heretic_test(288/h3s9-034 0:34 "") # heretic E3M9 SM Speed in 0:34 by Vincent Catalaa
add_heretic_test(289/h3s9-039 0:39 "") # heretic E3M9 SM Speed in 0:39 by Radek Pecka
add_heretic_test(625/h3s9-044 0:44 "") # heretic E3M9 SM Speed in 0:44 by Vincent Catalaa
add_heretic_test(542/h4f11215 12:15 "") # heretic E4M1 BP Max in 12:15 by Xit Vono
add_heretic_test(543/h4f11302 13:02 "") # heretic E4M1 BP Max in 13:02 by Vincent Catalaa
add_heretic_test(813/h4f1-815 8:15 "") # heretic E4M1 BP Max in 8:15 by Hitherto
add_heretic_test(290/h4b1-121 1:21 "") # heretic E4M1 BP Speed in 1:21 by Hitherto
add_heretic_test(291/h4b1-128 1:28 "") # heretic E4M1 BP Speed in 1:28 by Xit Vono
add_heretic_test(292/h4b1-135 1:35 "") # heretic E4M1 BP Speed in 1:35 by Vincent Catalaa
add_heretic_test(293/h4b1-216 2:16 "") # heretic E4M1 BP Speed in 2:16 by QWERTY
add_heretic_test(53456/h4x1-209 2:09 "") # heretic E4M1 NM 100S in 2:09 by JCD
add_heretic_test(314/h4n1-205 2:05 "") # heretic E4M1 NM Speed in 2:05 by Vincent Catalaa
add_heretic_test(78503/h4o1-5966 0:59 "") # heretic E4M1 NoMo in 0:59 by Ks4
add_heretic_test(77876/h4o110034 1:00 "") # heretic E4M1 NoMo in 1:00 by Ks4
add_heretic_test(65767/h4o1_10089 1:00 "") # heretic E4M1 NoMo in 1:00 by Batawi
add_heretic_test(814/h4o1-101 1:01 "") # heretic E4M1 NoMo in 1:01 by Hitherto
add_heretic_test(323/h4o1-103 1:03 "") # heretic E4M1 NoMo in 1:03 by JCD
add_heretic_test(565/h4p1-347 3:47 "") # heretic E4M1 Pacifist in 3:47 by Xit Vono
add_heretic_test(555/h4m11147 11:47 "") # heretic E4M1 SM Max in 11:47 by Vincent Catalaa
add_heretic_test(812/h4m1-715 7:15 "") # heretic E4M1 SM Max in 7:15 by Hitherto
add_heretic_test(73570/h4-1-11166 1:11 "") # heretic E4M1 SM Speed in 1:11 by Ks4
add_heretic_test(343/h4s1-115 1:15 "") # heretic E4M1 SM Speed in 1:15 by Hitherto
add_heretic_test(344/h4s1-138 1:38 "") # heretic E4M1 SM Speed in 1:38 by QWERTY
add_heretic_test(345/h4s1-239 2:39 "") # heretic E4M1 SM Speed in 2:39 by Vincent Catalaa
add_heretic_test(75840/h4f2-514 5:14 "") # heretic E4M2 BP Max in 5:14 by Doomniel
add_heretic_test(75586/h4f2-636 6:36 "") # heretic E4M2 BP Max in 6:36 by Doomniel
add_heretic_test(544/h4f2-757 7:57 "") # heretic E4M2 BP Max in 7:57 by Bruno Vergílio
add_heretic_test(294/h4b2-113 1:13 "") # heretic E4M2 BP Speed in 1:13 by QWERTY
add_heretic_test(295/h4b2-126 1:26 "") # heretic E4M2 BP Speed in 1:26 by Vincent Catalaa
add_heretic_test(296/h4b2-154 1:54 "") # heretic E4M2 BP Speed in 1:54 by QWERTY
add_heretic_test(297/h4b2-304 3:04 "") # heretic E4M2 BP Speed in 3:04 by Vincent Catalaa
add_heretic_test(315/h4n2-112 1:12 "") # heretic E4M2 NM Speed in 1:12 by Vincent Catalaa
add_heretic_test(669/h4o2-025 0:25 "-nomonsters") # heretic E4M2 NoMo in 0:25 by JCD
add_heretic_test(65796/h4o2_2631 0:26 "") # heretic E4M2 NoMo in 0:26 by Batawi
add_heretic_test(324/h4o2-030 0:30 "") # heretic E4M2 NoMo in 0:30 by JCD
add_heretic_test(325/h4o2-105 1:05 "") # heretic E4M2 NoMo in 1:05 by JCD
add_heretic_test(326/h4o2-108 1:08 "") # heretic E4M2 NoMo in 1:08 by PVS
add_heretic_test(338/h4p2-111 1:11 "") # heretic E4M2 Pacifist in 1:11 by Hitherto
add_heretic_test(566/h4p2-419 4:19 "") # heretic E4M2 Pacifist in 4:19 by Xit Vono
add_heretic_test(556/h4m21121 11:21 "") # heretic E4M2 SM Max in 11:21 by Bruno Vergílio
add_heretic_test(671/h4s2-026 0:26 "") # heretic E4M2 SM Speed in 0:26 by JCD
add_heretic_test(445/h4c2j029 0:29 "") # heretic E4M2 SM Speed in 0:29 by JCD, Ch0wW
add_heretic_test(346/h4s2-032 0:32 "") # heretic E4M2 SM Speed in 0:32 by JCD
add_heretic_test(347/h4s2-112 1:12 "") # heretic E4M2 SM Speed in 1:12 by QWERTY
add_heretic_test(348/h4s2-116 1:16 "") # heretic E4M2 SM Speed in 1:16 by Vincent Catalaa
add_heretic_test(349/h4s2-128 1:28 "") # heretic E4M2 SM Speed in 1:28 by Vincent Catalaa
add_heretic_test(626/h4s2-220 2:20 "") # heretic E4M2 SM Speed in 2:20 by Vincent Catalaa
add_heretic_test(627/h4s2-254 2:54 "") # heretic E4M2 SM Speed in 2:54 by Vincent Catalaa
add_heretic_test(545/h4f31416 14:16 "") # heretic E4M3 BP Max in 14:16 by Bruno Vergílio
add_heretic_test(298/h4b3-205 2:05 "") # heretic E4M3 BP Speed in 2:05 by QWERTY
add_heretic_test(299/h4b3-240 2:40 "") # heretic E4M3 BP Speed in 2:40 by Vincent Catalaa
add_heretic_test(53457/h4x3-235 2:35 "") # heretic E4M3 NM 100S in 2:35 by JCD
add_heretic_test(316/h4n3-222 2:22 "") # heretic E4M3 NM Speed in 2:22 by Vincent Catalaa
add_heretic_test(65797/h4o3_14560 1:45 "") # heretic E4M3 NoMo in 1:45 by Batawi
add_heretic_test(327/h4o3-146 1:46 "") # heretic E4M3 NoMo in 1:46 by JCD
add_heretic_test(613/h4p3-343 3:43 "") # heretic E4M3 Pacifist in 3:43 by PVS
add_heretic_test(557/h4m3-938 9:38 "") # heretic E4M3 SM Max in 9:38 by Bruno Vergílio
add_heretic_test(59856/h4c3-021 0:21 "") # heretic E4M3 SM Speed in 0:21 by Ch0wW, JCD
add_heretic_test(63744/h4s3-146 1:46 "") # heretic E4M3 SM Speed in 1:46 by JCD
add_heretic_test(350/h4s3-158 1:58 "") # heretic E4M3 SM Speed in 1:58 by Radek Pecka
add_heretic_test(351/h4s3-252 2:52 "") # heretic E4M3 SM Speed in 2:52 by Vincent Catalaa
add_heretic_test(546/h4f41106 11:06 "") # heretic E4M4 BP Max in 11:06 by Bruno Vergílio
add_heretic_test(300/h4b4-028 0:28 "") # heretic E4M4 BP Speed in 0:28 by Vincent Catalaa
add_heretic_test(301/h4b4-030 0:30 "") # heretic E4M4 BP Speed in 0:30 by QWERTY
add_heretic_test(302/h4b4-100 1:00 "") # heretic E4M4 BP Speed in 1:00 by Vincent Catalaa
add_heretic_test(53458/h4x4-136 1:36 "") # heretic E4M4 NM 100S in 1:36 by JCD
add_heretic_test(317/h4n4-028 0:28 "") # heretic E4M4 NM Speed in 0:28 by Vincent Catalaa
add_heretic_test(330/h4o42445 0:24 "") # heretic E4M4 NoMo in 0:24 by Kimo Xvirus
add_heretic_test(66215/h4o4_2523 0:25 "") # heretic E4M4 NoMo in 0:25 by Batawi
add_heretic_test(328/h4o42560 0:25 "") # heretic E4M4 NoMo in 0:25 by Vincent Catalaa
add_heretic_test(329/h4o42691 0:26 "-nomonsters") # heretic E4M4 NoMo in 0:26 by QWERTY
add_heretic_test(339/h4p4-033 0:33 "") # heretic E4M4 Pacifist in 0:33 by JCD
add_heretic_test(558/h4m4-906 9:06 "") # heretic E4M4 SM Max in 9:06 by Bruno Vergílio
add_heretic_test(352/h4s4-027 0:27 "") # heretic E4M4 SM Speed in 0:27 by Kimo Xvirus
add_heretic_test(353/h4s4-028 0:28 "") # heretic E4M4 SM Speed in 0:28 by Radek Pecka
add_heretic_test(354/h4s4-029 0:29 "") # heretic E4M4 SM Speed in 0:29 by QWERTY
add_heretic_test(355/h4s4-031 0:31 "") # heretic E4M4 SM Speed in 0:31 by QWERTY
add_heretic_test(356/h4s4-055 0:55 "") # heretic E4M4 SM Speed in 0:55 by Radek Pecka
add_heretic_test(357/h4s4-125 1:25 "") # heretic E4M4 SM Speed in 1:25 by Vincent Catalaa
add_heretic_test(303/h4b4s134 1:34 "") # heretic E4M4s BP Speed in 1:34 by Vincent Catalaa
add_heretic_test(318/h4n4s130 1:30 "") # heretic E4M4s NM Speed in 1:30 by JCD
add_heretic_test(66214/h4o4s_11491 1:14 "") # heretic E4M4s NoMo in 1:14 by Batawi
add_heretic_test(331/h4o4s115 1:15 "") # heretic E4M4s NoMo in 1:15 by JCD
add_heretic_test(340/h4p4s203 2:06 "") # heretic E4M4s Pacifist in 2:06 by Vincent Catalaa
add_heretic_test(65011/h4s4s119 1:19 "") # heretic E4M4s SM Speed in 1:19 by JCD
add_heretic_test(358/h4s4s143 1:43 "") # heretic E4M4s SM Speed in 1:43 by Vincent Catalaa
add_heretic_test(549/h4f51320 13:20 "") # heretic E4M5 BP Max in 13:20 by Bruno Vergílio
add_heretic_test(547/h4f5-906 9:06 "") # heretic E4M5 BP Max in 9:06 by Bruno Vergílio
add_heretic_test(548/h4f5-910 9:10 "") # heretic E4M5 BP Max in 9:10 by Xit Vono
add_heretic_test(304/h4b5-146 1:46 "") # heretic E4M5 BP Speed in 1:46 by Xit Vono
add_heretic_test(305/h4b5-200 2:00 "") # heretic E4M5 BP Speed in 2:00 by QWERTY
add_heretic_test(306/h4b5-217 2:17 "") # heretic E4M5 BP Speed in 2:17 by QWERTY
add_heretic_test(307/h4b5-255 2:55 "") # heretic E4M5 BP Speed in 2:55 by Vincent Catalaa
add_heretic_test(308/h4b5-430 4:30 "") # heretic E4M5 BP Speed in 4:30 by QWERTY
add_heretic_test(53459/h4x5-210 2:10 "") # heretic E4M5 NM 100S in 2:10 by JCD
add_heretic_test(55771/h4n5-130 1:30 "") # heretic E4M5 NM Speed in 1:30 by 4shockblast
add_heretic_test(319/h4n5-201 2:01 "") # heretic E4M5 NM Speed in 2:01 by Vincent Catalaa
add_heretic_test(66216/h4o5_10737 1:07 "") # heretic E4M5 NoMo in 1:07 by Batawi
add_heretic_test(332/h4o5-119 1:19 "") # heretic E4M5 NoMo in 1:19 by JCD
add_heretic_test(690/h4o5-122 1:22 "") # heretic E4M5 NoMo in 1:22 by JCD
add_heretic_test(341/h4p5-151 1:51 "") # heretic E4M5 Pacifist in 1:51 by Vincent Catalaa
add_heretic_test(342/h4p5-220 2:20 "") # heretic E4M5 Pacifist in 2:20 by Xit Vono
add_heretic_test(559/h4m51013 10:13 "") # heretic E4M5 SM Max in 10:13 by Bruno Vergílio
add_heretic_test(57009/heretice4m5-5437 0:54 "") # heretic E4M5 SM Speed in 0:54 by Ks4
add_heretic_test(56831/h4s5-108 1:08 "") # heretic E4M5 SM Speed in 1:08 by Mikolah
add_heretic_test(359/h4s5-135 1:35 "") # heretic E4M5 SM Speed in 1:35 by Radek Pecka
add_heretic_test(360/h4s5-159 1:59 "") # heretic E4M5 SM Speed in 1:59 by Vincent Catalaa
add_heretic_test(551/h4f61207 12:07 "") # heretic E4M6 BP Max in 12:07 by Bruno Vergílio
add_heretic_test(550/h4f6445 4:45 "") # heretic E4M6 BP Max in 4:45 by Mikhail Volkov
add_heretic_test(309/h4b6-120 1:20 "") # heretic E4M6 BP Speed in 1:20 by QWERTY
add_heretic_test(53460/h4x6-124 1:24 "") # heretic E4M6 NM 100S in 1:24 by JCD
add_heretic_test(320/h4n6-126 1:26 "") # heretic E4M6 NM Speed in 1:26 by JCD
add_heretic_test(67253/h4o6_5886 0:58 "") # heretic E4M6 NoMo in 0:58 by Batawi
add_heretic_test(333/h4o6-103 1:03 "") # heretic E4M6 NoMo in 1:03 by JCD
add_heretic_test(65010/h4p6-108 1:08 "") # heretic E4M6 Pacifist in 1:08 by JCD
add_heretic_test(55830/h4p6-125 1:25 "") # heretic E4M6 Pacifist in 1:25 by 4shockblast
add_heretic_test(614/h4p6-151 1:51 "") # heretic E4M6 Pacifist in 1:51 by PVS
add_heretic_test(560/h4m6-621 6:21 "") # heretic E4M6 SM Max in 6:21 by William Huber
add_heretic_test(59857/h4c6-032 0:32 "") # heretic E4M6 SM Speed in 0:32 by Ch0wW, JCD
add_heretic_test(54088/heretice4m6-11886 1:18 "") # heretic E4M6 SM Speed in 1:18 by Ks4
add_heretic_test(361/h4s6-124 1:24 "") # heretic E4M6 SM Speed in 1:24 by QWERTY
add_heretic_test(362/h4s6-154 1:54 "") # heretic E4M6 SM Speed in 1:54 by Vincent Catalaa
add_heretic_test(552/h4f71229 12:29 "") # heretic E4M7 BP Max in 12:29 by Bruno Vergílio
add_heretic_test(310/h4b7-141 1:41 "") # heretic E4M7 BP Speed in 1:41 by QWERTY
add_heretic_test(65683/h4x7-126 1:26 "") # heretic E4M7 NM 100S in 1:26 by JCD
add_heretic_test(55831/h4x7-133 1:33 "") # heretic E4M7 NM 100S in 1:33 by 4shockblast
add_heretic_test(53461/h4x7-144 1:44 "") # heretic E4M7 NM 100S in 1:44 by JCD
add_heretic_test(321/h4n7-141 1:41 "") # heretic E4M7 NM Speed in 1:41 by Vincent Catalaa
add_heretic_test(67254/h4o7_5871 0:58 "") # heretic E4M7 NoMo in 0:58 by Batawi
add_heretic_test(334/h4o7-105 1:05 "") # heretic E4M7 NoMo in 1:05 by JCD
add_heretic_test(335/h4o7-114 1:14 "-nomonsters") # heretic E4M7 NoMo in 1:14 by QWERTY
add_heretic_test(65718/h4p7-123 1:23 "") # heretic E4M7 Pacifist in 1:23 by JCD
add_heretic_test(670/h4p7-200 2:00 "") # heretic E4M7 Pacifist in 2:00 by PVS
add_heretic_test(561/h4m71041 10:41 "") # heretic E4M7 SM Max in 10:41 by Bruno Vergílio
add_heretic_test(63745/h4s7-107 1:07 "") # heretic E4M7 SM Speed in 1:07 by JCD
add_heretic_test(363/h4s7-121 1:21 "") # heretic E4M7 SM Speed in 1:21 by QWERTY
add_heretic_test(364/h4s7-251 2:51 "") # heretic E4M7 SM Speed in 2:51 by Vincent Catalaa
add_heretic_test(553/h4f8-344 3:44 "") # heretic E4M8 BP Max in 3:44 by Vincent Catalaa
add_heretic_test(311/h4b8-317 3:17 "") # heretic E4M8 BP Speed in 3:17 by QWERTY
add_heretic_test(312/h4b8-351 3:51 "") # heretic E4M8 BP Speed in 3:51 by QWERTY
add_heretic_test(53462/h4x8-212 2:12 "") # heretic E4M8 NM 100S in 2:12 by JCD
add_heretic_test(322/h4n8-207 2:07 "") # heretic E4M8 NM Speed in 2:07 by Vincent Catalaa
add_heretic_test(562/h4m8-350 3:50 "") # heretic E4M8 SM Max in 3:50 by Bruno Vergílio
add_heretic_test(563/h4m8-403 4:03 "") # heretic E4M8 SM Max in 4:03 by Bruno Vergílio
add_heretic_test(59858/h4c8-053 0:53 "") # heretic E4M8 SM Speed in 0:53 by Ch0wW, JCD
add_heretic_test(58232/heretice4m8-11154 1:11 "") # heretic E4M8 SM Speed in 1:11 by Ks4
add_heretic_test(365/h4s8-152 1:52 "") # heretic E4M8 SM Speed in 1:52 by QWERTY
add_heretic_test(366/h4s8-314 3:14 "") # heretic E4M8 SM Speed in 3:14 by Vincent Catalaa
add_heretic_test(554/h4f91318 13:18 "") # heretic E4M9 BP Max in 13:18 by Vincent Catalaa
add_heretic_test(55829/h4b9-202 2:02 "") # heretic E4M9 BP Speed in 2:02 by 4shockblast
add_heretic_test(313/h4b9-215 2:15 "") # heretic E4M9 BP Speed in 2:15 by QWERTY
add_heretic_test(53463/h4x9-311 3:11 "") # heretic E4M9 NM 100S in 3:11 by JCD
add_heretic_test(336/h4o9-134 1:34 "-nomonsters") # heretic E4M9 NoMo in 1:34 by JCD
add_heretic_test(67255/h4o9_13766 1:37 "") # heretic E4M9 NoMo in 1:37 by Batawi
add_heretic_test(337/h4o9-150 1:50 "") # heretic E4M9 NoMo in 1:50 by JCD
add_heretic_test(564/h4m91143 11:43 "") # heretic E4M9 SM Max in 11:43 by William Huber
add_heretic_test(65012/h4s9-154 1:54 "") # heretic E4M9 SM Speed in 1:54 by JCD
add_heretic_test(367/h4s9-219 2:19 "") # heretic E4M9 SM Speed in 2:19 by QWERTY
add_heretic_test(567/h4s9-349 3:49 "") # heretic E4M9 SM Speed in 3:49 by Vincent Catalaa
add_heretic_test(569/h5f11201 12:01 "") # heretic E5M1 BP Max in 12:01 by Bruno Vergílio
add_heretic_test(368/h5b1-201 2:01 "") # heretic E5M1 BP Speed in 2:01 by QWERTY
add_heretic_test(53465/h5x1-153 1:53 "") # heretic E5M1 NM 100S in 1:53 by JCD
add_heretic_test(55832/h5n1-137 1:37 "") # heretic E5M1 NM Speed in 1:37 by 4shockblast
add_heretic_test(378/h5n1-216 2:16 "") # heretic E5M1 NM Speed in 2:16 by JCD
add_heretic_test(76287/h5o15671 0:56 "") # heretic E5M1 NoMo in 0:56 by Ks4
add_heretic_test(73296/h5o1-5854 0:58 "") # heretic E5M1 NoMo in 0:58 by PrBoomerJuice
add_heretic_test(388/h5o1-059 0:59 "") # heretic E5M1 NoMo in 0:59 by JCD
add_heretic_test(578/h5m11108 11:08 "") # heretic E5M1 SM Max in 11:08 by Laurent Sebellin
add_heretic_test(579/h5m11419 14:19 "") # heretic E5M1 SM Max in 14:19 by Bruno Vergílio
add_heretic_test(62723/h5-1-11680 1:16 "") # heretic E5M1 SM Speed in 1:16 by Ks4
add_heretic_test(401/h5s1-157 1:57 "") # heretic E5M1 SM Speed in 1:57 by QWERTY
add_heretic_test(589/h5s1-412 4:12 "") # heretic E5M1 SM Speed in 4:12 by Vincent Catalaa
add_heretic_test(570/h5f2-744 7:44 "") # heretic E5M2 BP Max in 7:44 by Bruno Vergílio
add_heretic_test(54089/heretice5m2b-12343 1:23 "") # heretic E5M2 BP Speed in 1:23 by Ks4
add_heretic_test(369/h5b2-151 1:51 "") # heretic E5M2 BP Speed in 1:51 by QWERTY
add_heretic_test(53466/h5x2-147 1:47 "") # heretic E5M2 NM 100S in 1:47 by JCD
add_heretic_test(379/h5n2-148 1:48 "") # heretic E5M2 NM Speed in 1:48 by JCD
add_heretic_test(389/h5o2-056 0:56 "") # heretic E5M2 NoMo in 0:56 by JCD
add_heretic_test(390/h5o2-101 1:01 "") # heretic E5M2 NoMo in 1:01 by JCD
add_heretic_test(391/h5o2-108 1:08 "") # heretic E5M2 NoMo in 1:08 by PVS
add_heretic_test(55900/h5m2-552 5:52 "") # heretic E5M2 SM Max in 5:52 by 4shockblast
add_heretic_test(580/h5m2-744 7:44 "") # heretic E5M2 SM Max in 7:44 by Bruno Vergílio
add_heretic_test(450/35c2-041 0:41 "") # heretic E5M2 SM Speed in 0:41 by JCD, Dislogical, Oxyde
add_heretic_test(54180/heretice5m2-12240 1:22 "") # heretic E5M2 SM Speed in 1:22 by Ks4
add_heretic_test(402/h5s2-147 1:47 "") # heretic E5M2 SM Speed in 1:47 by QWERTY
add_heretic_test(590/h5s2-344 3:44 "") # heretic E5M2 SM Speed in 3:44 by Vincent Catalaa
add_heretic_test(571/h5f3-803 8:03 "") # heretic E5M3 BP Max in 8:03 by Vincent Catalaa
add_heretic_test(56477/heretice5m3b-3986 0:39 "") # heretic E5M3 BP Speed in 0:39 by Ks4
add_heretic_test(370/h5b3-042 0:42 "") # heretic E5M3 BP Speed in 0:42 by QWERTY
add_heretic_test(371/h5b3-138 1:38 "") # heretic E5M3 BP Speed in 1:38 by Vincent Catalaa
add_heretic_test(53467/h5x3-058 0:58 "") # heretic E5M3 NM 100S in 0:58 by JCD
add_heretic_test(380/h5n3-041 0:41 "") # heretic E5M3 NM Speed in 0:41 by JCD
add_heretic_test(381/h5n3-103 1:03 "") # heretic E5M3 NM Speed in 1:03 by Vincent Catalaa
add_heretic_test(392/h5o3-038 0:38 "-nomonsters") # heretic E5M3 NoMo in 0:38 by QWERTY
add_heretic_test(399/h5p3-045 0:45 "") # heretic E5M3 Pacifist in 0:45 by JCD
add_heretic_test(581/h5m3-727 7:27 "") # heretic E5M3 SM Max in 7:27 by Vincent Catalaa
add_heretic_test(446/h5c3c015 0:15 "") # heretic E5M3 SM Speed in 0:15 by JCD, Ch0wW
add_heretic_test(54181/heretice5m3-39,69 0:39 "") # heretic E5M3 SM Speed in 0:39 by Ks4
add_heretic_test(403/h5s3-041 0:41 "") # heretic E5M3 SM Speed in 0:41 by QWERTY
add_heretic_test(404/h5s3-043 0:43 "") # heretic E5M3 SM Speed in 0:43 by QWERTY
add_heretic_test(405/h5s3-120 1:20 "") # heretic E5M3 SM Speed in 1:20 by Vincent Catalaa
add_heretic_test(55717/h5b3s010 0:10 "") # heretic E5M3s BP Speed in 0:10 by 4shockblast
add_heretic_test(372/h5b3s051 0:51 "") # heretic E5M3s BP Speed in 0:51 by QWERTY
add_heretic_test(382/h5n3s055 0:55 "") # heretic E5M3s NM Speed in 0:55 by JCD
add_heretic_test(72100/h5o3s009 0:09 "") # heretic E5M3s NoMo in 0:09 by Insomnia
add_heretic_test(62595/h5s3so1137 0:11 "") # heretic E5M3s NoMo in 0:11 by AmnesiaSilence
add_heretic_test(393/h5o3s046 0:46 "-nomonsters") # heretic E5M3s NoMo in 0:46 by QWERTY
add_heretic_test(62593/h5p3s009 0:09 "") # heretic E5M3s Pacifist in 0:09 by aconfusedhuman
add_heretic_test(50925/h5p3s-010 0:10 "") # heretic E5M3s Pacifist in 0:10 by Thom Wye
add_heretic_test(50861/h5p3s010 0:10 "") # heretic E5M3s Pacifist in 0:10 by JCD
add_heretic_test(47539/h5p3s012 0:12 "") # heretic E5M3s Pacifist in 0:12 by JCD
add_heretic_test(400/h5p3s056 0:56 "") # heretic E5M3s Pacifist in 0:56 by JCD
add_heretic_test(62594/h5s3s-1243 0:12 "") # heretic E5M3s SM Speed in 0:12 by AmnesiaSilence
add_heretic_test(406/h5s3s049 0:49 "") # heretic E5M3s SM Speed in 0:49 by QWERTY
add_heretic_test(72101/h5o3sstr017 0:17 "") # heretic E5M3s Stroller in 0:17 by Insomnia
add_heretic_test(572/h5f4-930 9:30 "") # heretic E5M4 BP Max in 9:30 by Bruno Vergílio
add_heretic_test(373/h5b4-135 1:35 "") # heretic E5M4 BP Speed in 1:35 by QWERTY
add_heretic_test(53468/h5x4-149 1:49 "") # heretic E5M4 NM 100S in 1:49 by JCD
add_heretic_test(383/h5n4-136 1:36 "") # heretic E5M4 NM Speed in 1:36 by Vincent Catalaa
add_heretic_test(72102/h5o4-024 0:24 "") # heretic E5M4 NoMo in 0:24 by Insomnia
add_heretic_test(394/h5o4-027 0:27 "") # heretic E5M4 NoMo in 0:27 by JCD
add_heretic_test(582/h5m4-919 9:19 "") # heretic E5M4 SM Max in 9:19 by Bruno Vergílio
add_heretic_test(407/h5s4-030 0:30 "") # heretic E5M4 SM Speed in 0:30 by JCD
add_heretic_test(408/h5s4-132 1:32 "") # heretic E5M4 SM Speed in 1:32 by QWERTY
add_heretic_test(573/h5f52355 23:55 "") # heretic E5M5 BP Max in 23:55 by Laurent Sebellin
add_heretic_test(374/h5b5-253 2:53 "") # heretic E5M5 BP Speed in 2:53 by QWERTY
add_heretic_test(53469/h5x5-246 2:46 "") # heretic E5M5 NM 100S in 2:46 by JCD
add_heretic_test(384/h5n5-250 2:50 "") # heretic E5M5 NM Speed in 2:50 by JCD
add_heretic_test(395/h5o5-134 1:34 "") # heretic E5M5 NoMo in 1:34 by JCD
add_heretic_test(55718/h5s5os157 1:57 "") # heretic E5M5 NoMo 100S in 1:57 by 4shockblast
add_heretic_test(583/h5m51845 18:45 "") # heretic E5M5 SM Max in 18:45 by Bruno Vergílio
add_heretic_test(447/h5c5j051 0:51 "") # heretic E5M5 SM Speed in 0:51 by JCD, Ch0wW
add_heretic_test(65013/h5s5-154 1:54 "") # heretic E5M5 SM Speed in 1:54 by JCD
add_heretic_test(409/h5s5-235 2:35 "") # heretic E5M5 SM Speed in 2:35 by QWERTY
add_heretic_test(410/h5s5-309 3:09 "") # heretic E5M5 SM Speed in 3:09 by QWERTY
add_heretic_test(591/h5s5-548 5:48 "") # heretic E5M5 SM Speed in 5:48 by Vincent Catalaa
add_heretic_test(574/h5f61239 12:39 "") # heretic E5M6 BP Max in 12:39 by Bruno Vergílio
add_heretic_test(56479/heretice5m6b-13763 1:37 "") # heretic E5M6 BP Speed in 1:37 by Ks4
add_heretic_test(375/h5b6-219 2:19 "") # heretic E5M6 BP Speed in 2:19 by QWERTY
add_heretic_test(53470/h5x6-259 2:59 "") # heretic E5M6 NM 100S in 2:59 by JCD
add_heretic_test(73362/h5o6-10774 1:07 "") # heretic E5M6 NoMo in 1:07 by PrBoomerJuice
add_heretic_test(396/h5o6-114 1:14 "") # heretic E5M6 NoMo in 1:14 by JCD
add_heretic_test(584/h5m61157 11:57 "") # heretic E5M6 SM Max in 11:57 by Bruno Vergílio
add_heretic_test(56478/heretice5m6-13320 1:33 "") # heretic E5M6 SM Speed in 1:33 by Ks4
add_heretic_test(411/h5s6-152 1:52 "") # heretic E5M6 SM Speed in 1:52 by QWERTY
add_heretic_test(412/h5s6-215 2:15 "") # heretic E5M6 SM Speed in 2:15 by QWERTY
add_heretic_test(575/h5f71304 13:04 "") # heretic E5M7 BP Max in 13:04 by Vincent Catalaa
add_heretic_test(376/h5b7-226 2:26 "") # heretic E5M7 BP Speed in 2:26 by QWERTY
add_heretic_test(53471/h5x7-156 1:56 "") # heretic E5M7 NM 100S in 1:56 by JCD
add_heretic_test(385/h5n7-056 0:56 "") # heretic E5M7 NM Speed in 0:56 by JCD
add_heretic_test(386/h5n7-155 1:55 "") # heretic E5M7 NM Speed in 1:55 by JCD
add_heretic_test(387/h5n7-220 2:20 "") # heretic E5M7 NM Speed in 2:20 by Vincent Catalaa
add_heretic_test(73272/h5o74803 0:48 "") # heretic E5M7 NoMo in 0:48 by PrBoomerJuice
add_heretic_test(397/h5o74862 0:48 "") # heretic E5M7 NoMo in 0:48 by JCD
add_heretic_test(585/h5m71127 11:27 "") # heretic E5M7 SM Max in 11:27 by Bruno Vergílio
add_heretic_test(448/h5c7c030 0:30 "") # heretic E5M7 SM Speed in 0:30 by JCD, Ch0wW
add_heretic_test(73415/h5s7-4951 0:49 "") # heretic E5M7 SM Speed in 0:49 by PrBoomerJuice
add_heretic_test(413/h5s7-050 0:50 "") # heretic E5M7 SM Speed in 0:50 by JCD
add_heretic_test(691/h5s7-103 1:03 "") # heretic E5M7 SM Speed in 1:03 by JCD
add_heretic_test(414/h5s7-149 1:49 "") # heretic E5M7 SM Speed in 1:49 by JCD
add_heretic_test(415/h5s7-158 1:58 "") # heretic E5M7 SM Speed in 1:58 by QWERTY
add_heretic_test(576/h5f8-608 6:08 "") # heretic E5M8 BP Max in 6:08 by Vincent Catalaa
add_heretic_test(48821/h5n8-643 6:43 "") # heretic E5M8 NM Speed in 6:43 by JCD
add_heretic_test(58233/heretice5m8-22517 2:25 "") # heretic E5M8 SM Max in 2:25 by Ks4
add_heretic_test(416/h5s8-250 2:50 "") # heretic E5M8 SM Max in 2:50 by Vincent Catalaa
add_heretic_test(417/h5s8-336 3:36 "") # heretic E5M8 SM Max in 3:36 by JCD
add_heretic_test(586/h5m8-400 4:00 "") # heretic E5M8 SM Max in 4:00 by Vincent Catalaa
add_heretic_test(587/h5m8-409 4:09 "") # heretic E5M8 SM Max in 4:09 by Vincent Catalaa
add_heretic_test(674/hte5m8f 5:42 "") # heretic E5M8 SM Max in 5:42 by Savtchenko Denis
add_heretic_test(577/h5f91155 11:55 "") # heretic E5M9 BP Max in 11:55 by Vincent Catalaa
add_heretic_test(377/h5b9-218 2:18 "") # heretic E5M9 BP Speed in 2:18 by QWERTY
add_heretic_test(53464/h5n9-156 1:56 "") # heretic E5M9 NM Speed in 1:56 by JCD
add_heretic_test(398/h5o9-132 1:32 "") # heretic E5M9 NoMo in 1:32 by JCD
add_heretic_test(588/h5m91157 11:57 "") # heretic E5M9 SM Max in 11:57 by Vincent Catalaa
add_heretic_test(65014/h5s9-135 1:35 "") # heretic E5M9 SM Speed in 1:35 by JCD
add_heretic_test(418/h5s9-212 2:12 "") # heretic E5M9 SM Speed in 2:12 by QWERTY
add_heretic_test(55855/h6f1-453 4:53 "") # heretic E6M1 BP Max in 4:53 by 4shockblast
add_heretic_test(594/h6f1-510 5:10 "") # heretic E6M1 BP Max in 5:10 by Vincent Catalaa
add_heretic_test(419/h6b1-021 0:21 "") # heretic E6M1 BP Speed in 0:21 by Vincent Catalaa
add_heretic_test(420/h6b1-023 0:23 "") # heretic E6M1 BP Speed in 0:23 by QWERTY
add_heretic_test(74082/h6n1-2051 0:20 "") # heretic E6M1 NM Speed in 0:20 by PrBoomerJuice
add_heretic_test(53472/h6n1-020 0:20 "") # heretic E6M1 NM Speed in 0:20 by JCD
add_heretic_test(425/h6n1-022 0:22 "") # heretic E6M1 NM Speed in 0:22 by Vincent Catalaa
add_heretic_test(428/h6o11880 0:18 "") # heretic E6M1 NoMo in 0:18 by Kimo Xvirus
add_heretic_test(427/h6o11965 0:19 "-nomonsters") # heretic E6M1 NoMo in 0:19 by Vincent Catalaa
add_heretic_test(73536/h6p1-1983 0:19 "") # heretic E6M1 Pacifist in 0:19 by PrBoomerJuice
add_heretic_test(63418/h6p1-019 0:19 "") # heretic E6M1 Pacifist in 0:19 by JCD
add_heretic_test(433/h6s1-022 0:22 "") # heretic E6M1 Pacifist in 0:22 by Vincent Catalaa
add_heretic_test(430/h6p1-026 0:26 "") # heretic E6M1 Pacifist in 0:26 by JCD
add_heretic_test(596/h6m1-544 5:44 "") # heretic E6M1 SM Max in 5:44 by Vincent Catalaa
add_heretic_test(53474/heretice6m1-20 0:20 "") # heretic E6M1 SM Speed in 0:20 by Ks4
add_heretic_test(432/h6s1-021 0:21 "") # heretic E6M1 SM Speed in 0:21 by Vincent Catalaa
add_heretic_test(55901/h6f2-1110 11:10 "") # heretic E6M2 BP Max in 11:10 by 4shockblast
add_heretic_test(595/h6f21127 11:27 "") # heretic E6M2 BP Max in 11:27 by Vincent Catalaa
add_heretic_test(422/h6b2-130 1:30 "") # heretic E6M2 BP Speed in 1:30 by Vincent Catalaa
add_heretic_test(53473/h6n2-106 1:06 "") # heretic E6M2 NM Speed in 1:06 by JCD
add_heretic_test(426/h6n2-126 1:26 "") # heretic E6M2 NM Speed in 1:26 by Vincent Catalaa
add_heretic_test(429/h6o24739 0:47 "-nomonsters") # heretic E6M2 NoMo in 0:47 by Vincent Catalaa
add_heretic_test(431/h6p2-113 1:13 "") # heretic E6M2 Pacifist in 1:13 by QWERTY
add_heretic_test(597/h6m2-734 7:34 "") # heretic E6M2 SM Max in 7:34 by Vincent Catalaa
add_heretic_test(449/h6c2c006 0:06 "") # heretic E6M2 SM Speed in 0:06 by JCD, Ch0wW
add_heretic_test(56480/heretice6m2-5294 0:52 "") # heretic E6M2 SM Speed in 0:52 by Ks4
add_heretic_test(434/h6s2-056 0:56 "") # heretic E6M2 SM Speed in 0:56 by QWERTY
add_heretic_test(435/h6s2-113 1:13 "") # heretic E6M2 SM Speed in 1:13 by QWERTY
add_heretic_test(77405/demo-00251 5:49 "") # heretic Episode 1 BP Speed in 5:49 by Hanomamoru
add_heretic_test(52442/heretice1bp-646 6:46 "") # heretic Episode 1 BP Speed in 6:46 by Ks4
add_heretic_test(795/h1bs-734 7:34 "") # heretic Episode 1 BP Speed in 7:34 by veovis
add_heretic_test(601/h1bs-907 9:57 "") # heretic Episode 1 BP Speed in 9:57 by Vincent Catalaa
add_heretic_test(74454/h1nx1035 10:35 "") # heretic Episode 1 NM 100S in 10:35 by Insomnia
add_heretic_test(51771/h1nx1221 12:21 "") # heretic Episode 1 NM 100S in 12:21 by JCD
add_heretic_test(645/h1nm-836 10:19 "") # heretic Episode 1 NM Speed in 10:19 by JCD
add_heretic_test(644/h1nm-745 8:52 "") # heretic Episode 1 NM Speed in 8:52 by JCD
add_heretic_test(603/h1nm-811 9:03 "") # heretic Episode 1 NM Speed in 9:03 by Vincent Catalaa
add_heretic_test(74453/h1m-3333 33:33 "") # heretic Episode 1 SM Max in 33:33 by Insomnia
add_heretic_test(602/h1m-4825 52:40 "") # heretic Episode 1 SM Max in 52:40 by JCD
add_heretic_test(77050/hanoe1smdemo-522 5:22 "") # heretic Episode 1 SM Speed in 5:22 by Hanomamoru
add_heretic_test(70173/h1sp-525 5:25 "") # heretic Episode 1 SM Speed in 5:25 by Thom Wye
add_heretic_test(68076/h1sp-532 5:32 "") # heretic Episode 1 SM Speed in 5:32 by Thom Wye
add_heretic_test(67355/h1sp-537 5:37 "") # heretic Episode 1 SM Speed in 5:37 by Thom Wye
add_heretic_test(52437/h1sp-539 5:39 "") # heretic Episode 1 SM Speed in 5:39 by Zero-Master
add_heretic_test(64003/h1sp-541 5:41 "") # heretic Episode 1 SM Speed in 5:41 by Thom Wye
add_heretic_test(800/h1sp-521 5:52 "") # heretic Episode 1 SM Speed in 5:52 by JCD
add_heretic_test(50933/heretice1-556 5:56 "") # heretic Episode 1 SM Speed in 5:56 by Ks4
add_heretic_test(50860/heretice1-610 6:10 "") # heretic Episode 1 SM Speed in 6:10 by Ks4
add_heretic_test(598/21spj558 6:35 "") # heretic Episode 1 SM Speed in 6:35 by JCD, Ch0wW
add_heretic_test(799/srt2 6:49 "") # heretic Episode 1 SM Speed in 6:49 by Xindage
add_heretic_test(612/h1sp-622 6:54 "") # heretic Episode 1 SM Speed in 6:54 by veovis
add_heretic_test(646/h1sp-643 8:04 "") # heretic Episode 1 SM Speed in 8:04 by JCD
add_heretic_test(647/h1sp-721 8:07 "") # heretic Episode 1 SM Speed in 8:07 by JCD
add_heretic_test(648/h1sp-753 8:52 "") # heretic Episode 1 SM Speed in 8:52 by JCD
add_heretic_test(649/h1sp-826 9:40 "") # heretic Episode 1 SM Speed in 9:40 by Vincent Catalaa
add_heretic_test(604/h2bs1104 14:29 "") # heretic Episode 2 BP Speed in 14:29 by Vincent Catalaa
add_heretic_test(797/h2bs-728 7:28 "") # heretic Episode 2 BP Speed in 7:28 by veovis
add_heretic_test(55504/h2nx1438 14:38 "") # heretic Episode 2 NM 100S in 14:38 by JCD
add_heretic_test(50282/heretic_e2_pacifist_20_20 20:20 "") # heretic Episode 2 Pacifist in 20:20 by Thom Wye
add_heretic_test(605/h2ma6048 67:02 "") # heretic Episode 2 SM Max in 67:02 by JCD
add_heretic_test(75354/h2m-3658 36:58 "") # heretic Episode 2 SM Max in 36:58 by Insomnia
add_heretic_test(73074/h2sp-558 5:58 "") # heretic Episode 2 SM Speed in 5:58 by Thom Wye
add_heretic_test(599/22spj504 6:07 "") # heretic Episode 2 SM Speed in 6:07 by JCD, Ch0wW
add_heretic_test(52438/h2sp-616 6:16 "") # heretic Episode 2 SM Speed in 6:16 by Zero-Master
add_heretic_test(50991/heretice2-645 6:45 "") # heretic Episode 2 SM Speed in 6:45 by Ks4
add_heretic_test(722/h2sp-649 6:49 "") # heretic Episode 2 SM Speed in 6:49 by veovis
add_heretic_test(650/h2sp-619 7:38 "") # heretic Episode 2 SM Speed in 7:38 by Hitherto
add_heretic_test(651/h2sp-746 8:54 "") # heretic Episode 2 SM Speed in 8:54 by JCD
add_heretic_test(652/h2sp-752 9:20 "") # heretic Episode 2 SM Speed in 9:20 by Vincent Catalaa
add_heretic_test(55516/h3nx1744 17:44 "") # heretic Episode 3 NM 100S in 17:44 by JCD
add_heretic_test(606/h3ma5933 62:48 "") # heretic Episode 3 SM Max in 62:48 by JCD
add_heretic_test(76682/h3m-4708 47:08 "") # heretic Episode 3 SM Max in 47:08 by Insomnia
add_heretic_test(607/h3sp-827 10:39 "") # heretic Episode 3 SM Speed in 10:39 by Vincent Catalaa
add_heretic_test(608/h3sp-829 11:15 "") # heretic Episode 3 SM Speed in 11:15 by Lucas Marincak
add_heretic_test(52439/h3sp-758 7:58 "") # heretic Episode 3 SM Speed in 7:58 by Zero-Master
add_heretic_test(723/h3sp-828 8:28 "") # heretic Episode 3 SM Speed in 8:28 by veovis
add_heretic_test(73599/h3sp-835 8:35 "") # heretic Episode 3 SM Speed in 8:35 by Ks4
add_heretic_test(653/h3sp-707 8:50 "") # heretic Episode 3 SM Speed in 8:50 by veovis
add_heretic_test(53455/h4nx2105 21:05 "") # heretic Episode 4 NM 100S in 21:05 by JCD
add_heretic_test(724/h4sp1055 10:55 "") # heretic Episode 4 SM Speed in 10:55 by veovis
add_heretic_test(609/h4sp1051 12:08 "") # heretic Episode 4 SM Speed in 12:08 by Vincent Catalaa
add_heretic_test(52440/h4sp-704 7:04 "") # heretic Episode 4 SM Speed in 7:04 by Zero-Master
add_heretic_test(51068/heretice4-755 7:55 "") # heretic Episode 4 SM Speed in 7:55 by Ks4
add_heretic_test(55684/h5nx2604 26:04 "") # heretic Episode 5 NM 100S in 26:04 by JCD
add_heretic_test(56476/heretice5-1045 10:45 "") # heretic Episode 5 SM Speed in 10:45 by Ks4
add_heretic_test(51288/heretice5-1116 11:16 "") # heretic Episode 5 SM Speed in 11:16 by Ks4
add_heretic_test(796/h5sp1257 12:57 "") # heretic Episode 5 SM Speed in 12:57 by veovis
add_heretic_test(725/h5sp1413 14:13 "") # heretic Episode 5 SM Speed in 14:13 by veovis
add_heretic_test(610/h5sp1252 16:48 "") # heretic Episode 5 SM Speed in 16:48 by Vincent Catalaa
add_heretic_test(611/h5sp1407 17:26 "") # heretic Episode 5 SM Speed in 17:26 by Vincent Catalaa
add_heretic_test(52441/h5sp-930 9:30 "") # heretic Episode 5 SM Speed in 9:30 by Zero-Master
add_heretic_test(50734/h2os1032 10:28 "") # heretic Other Movie NoMo 100S in 10:28 by PVS
add_heretic_test(50733/h1os1109 11:05 "") # heretic Other Movie NoMo 100S in 11:05 by PVS
