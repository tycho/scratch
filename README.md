# High resolution sleep

The STL provides the function `std::this_thread::sleep()`, which supports very fine-grained (nanosecond-granularity) sleeps on non-Windows platforms. On Windows, however, it is implemented (in both Microsoft's STL and in libc++) using the Win32 `Sleep()` API, which is millisecond granularity. It's also not very punctual as sleep APIs go.

On Windows 10 build 17134 or later, the `CreateWaitableTimerEx()` API got support for a new `CREATE_WAITABLE_TIMER_HIGH_RESOLUTION` flag, which allows setting timers with sub-millisecond granularity. Sleep can be implemented using this API. 

I've implemented a couple of simple `std::this_thread::sleep()` alternatives which are more disciplined about their behavior:

- **timers::deadline_sleep** - Attempts to sleep *exactly* the requested duration. To accomplish this, it uses an "adaptive sleep" mechanism, where it sleeps for as long as it feels it can without overshooting the deadline, and then spin-waits for the rest of the duration.
- **timers::precise_sleep** - Attempts to sleep using the highest-precision sleep API available on the OS. For Windows, this means either a high resolution waitable timer or plain old Sleep().

### Results

The `main()` function in this program will run through some exercises of various sleep durations and measure how precise the actual sleep duration was.

Here's a run on Windows 11 (22621.1848):

```
Windows timer resolution minimum 15625 us, maximum 500 us, current 500 us
Windows timer resolution set to 500 us

wait time: 50000 us
  std::this_thread::sleep_for
    elapsed: 50379 us, error: 379 us (0.76%)
    elapsed: 50469 us, error: 469 us (0.94%)
    elapsed: 50024 us, error: 24 us (0.05%)
    elapsed: 50508 us, error: 508 us (1.02%)
    elapsed: 50023 us, error: 23 us (0.05%)
  timers::precise_sleep::sleep_for
    elapsed: 50163 us, error: 163 us (0.33%)
    elapsed: 50020 us, error: 20 us (0.04%)
    elapsed: 50527 us, error: 527 us (1.05%)
    elapsed: 50373 us, error: 373 us (0.75%)
    elapsed: 50434 us, error: 434 us (0.87%)
  timers::deadline_sleep::sleep_for
    elapsed: 50002 us, error: 2 us (0.00%)
    elapsed: 50002 us, error: 2 us (0.00%)
    elapsed: 50001 us, error: 1 us (0.00%)
    elapsed: 50002 us, error: 2 us (0.00%)
    elapsed: 50002 us, error: 2 us (0.00%)

wait time: 20000 us
  std::this_thread::sleep_for
    elapsed: 20127 us, error: 127 us (0.64%)
    elapsed: 20014 us, error: 14 us (0.07%)
    elapsed: 20506 us, error: 506 us (2.53%)
    elapsed: 20482 us, error: 482 us (2.41%)
    elapsed: 20012 us, error: 12 us (0.06%)
  timers::precise_sleep::sleep_for
    elapsed: 20408 us, error: 408 us (2.04%)
    elapsed: 20011 us, error: 11 us (0.06%)
    elapsed: 20476 us, error: 476 us (2.38%)
    elapsed: 20331 us, error: 331 us (1.65%)
    elapsed: 20146 us, error: 146 us (0.73%)
  timers::deadline_sleep::sleep_for
    elapsed: 20000 us, error: 0 us (0.00%)
    elapsed: 20000 us, error: 0 us (0.00%)
    elapsed: 20000 us, error: 0 us (0.00%)
    elapsed: 20000 us, error: 0 us (0.00%)
    elapsed: 20000 us, error: 0 us (0.00%)

wait time: 10000 us
  std::this_thread::sleep_for
    elapsed: 10074 us, error: 74 us (0.74%)
    elapsed: 10482 us, error: 482 us (4.82%)
    elapsed: 10012 us, error: 12 us (0.12%)
    elapsed: 10007 us, error: 7 us (0.07%)
    elapsed: 10431 us, error: 431 us (4.31%)
  timers::precise_sleep::sleep_for
    elapsed: 10429 us, error: 429 us (4.29%)
    elapsed: 10157 us, error: 157 us (1.57%)
    elapsed: 10010 us, error: 10 us (0.10%)
    elapsed: 10253 us, error: 253 us (2.53%)
    elapsed: 10490 us, error: 490 us (4.90%)
  timers::deadline_sleep::sleep_for
    elapsed: 10000 us, error: 0 us (0.00%)
    elapsed: 10000 us, error: 0 us (0.00%)
    elapsed: 10000 us, error: 0 us (0.00%)
    elapsed: 10000 us, error: 0 us (0.00%)
    elapsed: 10001 us, error: 1 us (0.01%)

wait time: 5000 us
  std::this_thread::sleep_for
    elapsed: 5151 us, error: 151 us (3.02%)
    elapsed: 5476 us, error: 476 us (9.52%)
    elapsed: 5471 us, error: 471 us (9.42%)
    elapsed: 5469 us, error: 469 us (9.38%)
    elapsed: 5473 us, error: 473 us (9.46%)
  timers::precise_sleep::sleep_for
    elapsed: 5017 us, error: 17 us (0.34%)
    elapsed: 5476 us, error: 476 us (9.52%)
    elapsed: 5487 us, error: 487 us (9.74%)
    elapsed: 5444 us, error: 444 us (8.88%)
    elapsed: 5009 us, error: 9 us (0.18%)
  timers::deadline_sleep::sleep_for
    elapsed: 5000 us, error: 0 us (0.00%)
    elapsed: 5000 us, error: 0 us (0.00%)
    elapsed: 5000 us, error: 0 us (0.00%)
    elapsed: 5000 us, error: 0 us (0.00%)
    elapsed: 5000 us, error: 0 us (0.00%)

wait time: 1000 us
  std::this_thread::sleep_for
    elapsed: 1013 us, error: 13 us (1.30%)
    elapsed: 1010 us, error: 10 us (1.00%)
    elapsed: 1486 us, error: 486 us (48.60%)
    elapsed: 1459 us, error: 459 us (45.90%)
    elapsed: 1014 us, error: 14 us (1.40%)
  timers::precise_sleep::sleep_for
    elapsed: 1013 us, error: 13 us (1.30%)
    elapsed: 1009 us, error: 9 us (0.90%)
    elapsed: 1010 us, error: 10 us (1.00%)
    elapsed: 1006 us, error: 6 us (0.60%)
    elapsed: 1008 us, error: 8 us (0.80%)
  timers::deadline_sleep::sleep_for
    elapsed: 1000 us, error: 0 us (0.00%)
    elapsed: 1000 us, error: 0 us (0.00%)
    elapsed: 1000 us, error: 0 us (0.00%)
    elapsed: 1000 us, error: 0 us (0.00%)
    elapsed: 1000 us, error: 0 us (0.00%)

wait time: 500 us
  std::this_thread::sleep_for
    elapsed: 1306 us, error: 806 us (161.20%)
    elapsed: 1467 us, error: 967 us (193.40%)
    elapsed: 1472 us, error: 972 us (194.40%)
    elapsed: 1471 us, error: 971 us (194.20%)
    elapsed: 1476 us, error: 976 us (195.20%)
  timers::precise_sleep::sleep_for
    elapsed: 515 us, error: 15 us (3.00%)
    elapsed: 524 us, error: 24 us (4.80%)
    elapsed: 512 us, error: 12 us (2.40%)
    elapsed: 516 us, error: 16 us (3.20%)
    elapsed: 512 us, error: 12 us (2.40%)
  timers::deadline_sleep::sleep_for
    elapsed: 500 us, error: 0 us (0.00%)
    elapsed: 500 us, error: 0 us (0.00%)
    elapsed: 500 us, error: 0 us (0.00%)
    elapsed: 500 us, error: 0 us (0.00%)
    elapsed: 500 us, error: 0 us (0.00%)

wait time: 100 us
  std::this_thread::sleep_for
    elapsed: 1253 us, error: 1153 us (1153.00%)
    elapsed: 1464 us, error: 1364 us (1364.00%)
    elapsed: 1476 us, error: 1376 us (1376.00%)
    elapsed: 1473 us, error: 1373 us (1373.00%)
    elapsed: 1465 us, error: 1365 us (1365.00%)
  timers::precise_sleep::sleep_for
    elapsed: 447 us, error: 347 us (347.00%)
    elapsed: 474 us, error: 374 us (374.00%)
    elapsed: 467 us, error: 367 us (367.00%)
    elapsed: 479 us, error: 379 us (379.00%)
    elapsed: 461 us, error: 361 us (361.00%)
  timers::deadline_sleep::sleep_for
    elapsed: 100 us, error: 0 us (0.00%)
    elapsed: 100 us, error: 0 us (0.00%)
    elapsed: 100 us, error: 0 us (0.00%)
    elapsed: 100 us, error: 0 us (0.00%)
    elapsed: 100 us, error: 0 us (0.00%)

wait time: 1 us
  std::this_thread::sleep_for
    elapsed: 1309 us, error: 1308 us (130800.00%)
    elapsed: 1009 us, error: 1008 us (100800.00%)
    elapsed: 1473 us, error: 1472 us (147200.00%)
    elapsed: 1481 us, error: 1480 us (148000.00%)
    elapsed: 1009 us, error: 1008 us (100800.00%)
  timers::precise_sleep::sleep_for
    elapsed: 8 us, error: 7 us (700.00%)
    elapsed: 500 us, error: 499 us (49900.00%)
    elapsed: 487 us, error: 486 us (48600.00%)
    elapsed: 483 us, error: 482 us (48200.00%)
    elapsed: 497 us, error: 496 us (49600.00%)
  timers::deadline_sleep::sleep_for
    elapsed: 1 us, error: 0 us (0.00%)
    elapsed: 1 us, error: 0 us (0.00%)
    elapsed: 1 us, error: 0 us (0.00%)
    elapsed: 1 us, error: 0 us (0.00%)
    elapsed: 1 us, error: 0 us (0.00%)

wait time: 100 us
  std::this_thread::sleep_for
    elapsed: 1012 us, error: 912 us (912.00%)
    elapsed: 1476 us, error: 1376 us (1376.00%)
    elapsed: 1007 us, error: 907 us (907.00%)
    elapsed: 1977 us, error: 1877 us (1877.00%)
    elapsed: 1477 us, error: 1377 us (1377.00%)
  timers::precise_sleep::sleep_for
    elapsed: 451 us, error: 351 us (351.00%)
    elapsed: 484 us, error: 384 us (384.00%)
    elapsed: 472 us, error: 372 us (372.00%)
    elapsed: 466 us, error: 366 us (366.00%)
    elapsed: 468 us, error: 368 us (368.00%)
  timers::deadline_sleep::sleep_for
    elapsed: 100 us, error: 0 us (0.00%)
    elapsed: 100 us, error: 0 us (0.00%)
    elapsed: 100 us, error: 0 us (0.00%)
    elapsed: 100 us, error: 0 us (0.00%)
    elapsed: 100 us, error: 0 us (0.00%)

wait time: 500 us
  std::this_thread::sleep_for
    elapsed: 1293 us, error: 793 us (158.60%)
    elapsed: 1469 us, error: 969 us (193.80%)
    elapsed: 1473 us, error: 973 us (194.60%)
    elapsed: 1482 us, error: 982 us (196.40%)
    elapsed: 1467 us, error: 967 us (193.40%)
  timers::precise_sleep::sleep_for
    elapsed: 512 us, error: 12 us (2.40%)
    elapsed: 512 us, error: 12 us (2.40%)
    elapsed: 514 us, error: 14 us (2.80%)
    elapsed: 513 us, error: 13 us (2.60%)
    elapsed: 510 us, error: 10 us (2.00%)
  timers::deadline_sleep::sleep_for
    elapsed: 500 us, error: 0 us (0.00%)
    elapsed: 500 us, error: 0 us (0.00%)
    elapsed: 500 us, error: 0 us (0.00%)
    elapsed: 500 us, error: 0 us (0.00%)
    elapsed: 500 us, error: 0 us (0.00%)

wait time: 1000 us
  std::this_thread::sleep_for
    elapsed: 1273 us, error: 273 us (27.30%)
    elapsed: 1475 us, error: 475 us (47.50%)
    elapsed: 1478 us, error: 478 us (47.80%)
    elapsed: 1469 us, error: 469 us (46.90%)
    elapsed: 1491 us, error: 491 us (49.10%)
  timers::precise_sleep::sleep_for
    elapsed: 1020 us, error: 20 us (2.00%)
    elapsed: 1014 us, error: 14 us (1.40%)
    elapsed: 1472 us, error: 472 us (47.20%)
    elapsed: 1013 us, error: 13 us (1.30%)
    elapsed: 1020 us, error: 20 us (2.00%)
  timers::deadline_sleep::sleep_for
    elapsed: 1000 us, error: 0 us (0.00%)
    elapsed: 1000 us, error: 0 us (0.00%)
    elapsed: 1000 us, error: 0 us (0.00%)
    elapsed: 1000 us, error: 0 us (0.00%)
    elapsed: 1000 us, error: 0 us (0.00%)

wait time: 5000 us
  std::this_thread::sleep_for
    elapsed: 5212 us, error: 212 us (4.24%)
    elapsed: 5287 us, error: 287 us (5.74%)
    elapsed: 5095 us, error: 95 us (1.90%)
    elapsed: 5455 us, error: 455 us (9.10%)
    elapsed: 5483 us, error: 483 us (9.66%)
  timers::precise_sleep::sleep_for
    elapsed: 5021 us, error: 21 us (0.42%)
    elapsed: 5012 us, error: 12 us (0.24%)
    elapsed: 5377 us, error: 377 us (7.54%)
    elapsed: 5494 us, error: 494 us (9.88%)
    elapsed: 5022 us, error: 22 us (0.44%)
  timers::deadline_sleep::sleep_for
    elapsed: 5000 us, error: 0 us (0.00%)
    elapsed: 5000 us, error: 0 us (0.00%)
    elapsed: 5000 us, error: 0 us (0.00%)
    elapsed: 5000 us, error: 0 us (0.00%)
    elapsed: 5000 us, error: 0 us (0.00%)

wait time: 10000 us
  std::this_thread::sleep_for
    elapsed: 10296 us, error: 296 us (2.96%)
    elapsed: 10480 us, error: 480 us (4.80%)
    elapsed: 10495 us, error: 495 us (4.95%)
    elapsed: 10271 us, error: 271 us (2.71%)
    elapsed: 10483 us, error: 483 us (4.83%)
  timers::precise_sleep::sleep_for
    elapsed: 10041 us, error: 41 us (0.41%)
    elapsed: 10440 us, error: 440 us (4.40%)
    elapsed: 10445 us, error: 445 us (4.45%)
    elapsed: 10474 us, error: 474 us (4.74%)
    elapsed: 10494 us, error: 494 us (4.94%)
  timers::deadline_sleep::sleep_for
    elapsed: 10000 us, error: 0 us (0.00%)
    elapsed: 10000 us, error: 0 us (0.00%)
    elapsed: 10000 us, error: 0 us (0.00%)
    elapsed: 10001 us, error: 1 us (0.01%)
    elapsed: 10000 us, error: 0 us (0.00%)

wait time: 20000 us
  std::this_thread::sleep_for
    elapsed: 20442 us, error: 442 us (2.21%)
    elapsed: 20016 us, error: 16 us (0.08%)
    elapsed: 20451 us, error: 451 us (2.25%)
    elapsed: 20039 us, error: 39 us (0.19%)
    elapsed: 20266 us, error: 266 us (1.33%)
  timers::precise_sleep::sleep_for
    elapsed: 20011 us, error: 11 us (0.06%)
    elapsed: 20331 us, error: 331 us (1.65%)
    elapsed: 20511 us, error: 511 us (2.56%)
    elapsed: 20257 us, error: 257 us (1.29%)
    elapsed: 20507 us, error: 507 us (2.54%)
  timers::deadline_sleep::sleep_for
    elapsed: 20001 us, error: 1 us (0.01%)
    elapsed: 20001 us, error: 1 us (0.01%)
    elapsed: 20000 us, error: 0 us (0.00%)
    elapsed: 20000 us, error: 0 us (0.00%)
    elapsed: 20000 us, error: 0 us (0.00%)

wait time: 50000 us
  std::this_thread::sleep_for
    elapsed: 50111 us, error: 111 us (0.22%)
    elapsed: 50504 us, error: 504 us (1.01%)
    elapsed: 50262 us, error: 262 us (0.52%)
    elapsed: 50513 us, error: 513 us (1.03%)
    elapsed: 50110 us, error: 110 us (0.22%)
  timers::precise_sleep::sleep_for
    elapsed: 50133 us, error: 133 us (0.27%)
    elapsed: 50340 us, error: 340 us (0.68%)
    elapsed: 50356 us, error: 356 us (0.71%)
    elapsed: 50060 us, error: 60 us (0.12%)
    elapsed: 50095 us, error: 95 us (0.19%)
  timers::deadline_sleep::sleep_for
    elapsed: 50004 us, error: 4 us (0.01%)
    elapsed: 50003 us, error: 3 us (0.01%)
    elapsed: 50003 us, error: 3 us (0.01%)
    elapsed: 50001 us, error: 1 us (0.00%)
    elapsed: 50001 us, error: 1 us (0.00%)
```
