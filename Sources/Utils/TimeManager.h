#ifndef _TimeManager_h_
#define _TimeManager_h_

typedef double Second;

typedef struct {
    Second dt;
    Second seconds; // Seconds since simulation begins
    int steps; // Steps since simulation begins
} Clock;

class TimeManager
{
public:
    TimeManager();
    ~TimeManager();

    static bool onLine() { return isOnLine; }
    static void reset() { isClockSet = false; }
    static void setClock(Second dt, Second seconds = 0.0, int steps = 0);
    static bool isFirstStep() { return clock.steps == startStep; }
    static bool isFinished() { return clock.steps >= endStep; }
    static Second getSeconds() { return clock.seconds; }
    static Second getTimeStep() { return clock.dt; }
    static int getSteps() { return clock.steps; }
    static void setEndStep(int);
    static void advance();

protected:
    static bool isOnLine;
    static bool isClockSet;
    static Clock clock;
    static int startStep;
    static int endStep;
};

#endif
