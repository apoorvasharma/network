#Ricart Agarwala Algorithm Implementation

#### A demonstartion of Ricart Agarwala algorithm, using Lamport's Logial clock for single Resource
Each device established a *TCP connection* with every other device. When trying to accedd a resource they deviced pass messages amongst themselves and decide who gets the preference. A *remote server* which writes down whatever messages it gets in a file to simulates a ctitical reource.

I have preferred threads over using *epoll* or *select* to provide better esponse time and take advantage of CPUs which can run more threads

Note: Might require minor changes to work for beyond 10 devices.
