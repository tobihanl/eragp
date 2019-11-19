#ifndef EVOLUTION_LOG_H
#define EVOLUTION_LOG_H

#include <chrono>
#include <cstdio>

typedef struct {
    int turn = -1;
    int food = -1;
    int livings = -1;
    int mpi = -1;
    int tick = -1;
    int render = -1;
    int delay = -1;
    int overall = -1;
} LogData;

class Log {
private:
    static bool logging;
    static FILE *logFile;

public:
    static LogData data;

private:
    static void writeLine(const std::string &line) {
        if (!logging || line.length() == 0) return;

        fwrite(line.c_str(), sizeof(char), line.length(), logFile);

        // Begin new line (when not written)
        if ('\n' != *line.rbegin())
            fwrite("\n", sizeof(char), 1, logFile);
    }

public:
    static void startLogging(const std::string &file) {
        if (logging) return;

        logFile = fopen(file.c_str(), "a");
        if (logFile) {
            logging = true;
            writeLine("Turn,Food,Livings,MPI,Tick,Render,Delay,Overall");
        }
    }

    static void endLogging() {
        if (logging) {
            logging = false;
            fclose(logFile);
            logFile = nullptr;
        }
    }

    static void writeLogData() {
        if (!logging) return;

        // Write line of logging data
        writeLine(std::to_string(data.turn) + "," + std::to_string(data.food) + "," +
                  std::to_string(data.livings) + "," + std::to_string(data.mpi) + "," +
                  std::to_string(data.tick) + "," + std::to_string(data.render) + "," +
                  std::to_string(data.delay) + "," + std::to_string(data.overall));

        // Reset logging structure
        data = {};
    }

    static int currentTime() {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count();
    }

    static int endTime(const int &start) {
        return (currentTime() - start);
    }
};

#endif //EVOLUTION_LOG_H
