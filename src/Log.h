#ifndef EVOLUTION_LOG_H
#define EVOLUTION_LOG_H

#include <chrono>
#include <cstdio>

class Log {
private:
    static bool logging;
    static FILE *logFile;

public:
    static void startLogging(const std::string &file) {
        if (logging) return;

        logFile = fopen(file.c_str(), "a");
        if (logFile) logging = true;
    }

    static void endLogging() {
        if (logging) {
            logging = false;
            fclose(logFile);
            logFile = nullptr;
        }
    }

    static void writeLine(const std::string &line) {
        if (!logging || line.length() == 0) return;

        fwrite(line.c_str(), sizeof(char), line.length(), logFile);

        // Begin new line (when not written)
        if ('\n' != *line.rbegin())
            fwrite("\n", sizeof(char), 1, logFile);
    }

    static int currentTime() {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count();
    }

    static int endTime(const int &start) {
        return (currentTime() - start);
    }
};

bool Log::logging = false;
FILE *Log::logFile = nullptr;

#endif //EVOLUTION_LOG_H
