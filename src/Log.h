#ifndef EVOLUTION_LOG_H
#define EVOLUTION_LOG_H

#include <chrono>
#include <cstdio>
#include "Constants.h"
#include "Structs.h"

class Log {
private:
    static bool paused;
    static bool logging;
    static FILE *logFile;

    static int occupied;
    static LogData buffer[LOG_DATA_BUFFER_SIZE];

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

    static void writeBackLogData() {
        if (!logging) return;

        for (int i = 0; i < occupied; i++) {
            LogData logData = buffer[i];
            writeLine(std::to_string(logData.turn) + "," + std::to_string(logData.food) + "," +
                      std::to_string(logData.livings) + "," + std::to_string(logData.mpi) + "," +
                      std::to_string(logData.think) + "," +
                      std::to_string(logData.tick) + "," + std::to_string(logData.render) + "," +
                      std::to_string(logData.delay) + "," + std::to_string(logData.overall));
        }

        occupied = 0;
    }

public:
    static void startLogging(const std::string &file) {
        if (logging) return;

        logFile = fopen(file.c_str(), "a");
        if (logFile) {
            logging = true;
            writeLine("sep=,"); // Specify separator
            writeLine("Turn,Food,Livings,MPI,Think,Tick,Render,Delay,Overall");
        }
    }

    static void endLogging() {
        if (logging) {
            writeBackLogData();
            fclose(logFile);
            logging = false;
            logFile = nullptr;
        }
    }

    static void saveLogData() {
        if (!logging) return;

        buffer[occupied++] = data;
        data = {};

        if (occupied == LOG_DATA_BUFFER_SIZE)
            writeBackLogData();
    }

    static void enable() {
        paused = false;
    }

    static void disable() {
        paused = true;
    }

    static bool isEnabled() {
        return logging && !paused;
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
