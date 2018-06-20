#include "log.hpp"

#include <string>
#include <vector>
#include <memory>
#include <unistd.h>
#include <sys/stat.h>

#include <stdio.h>
#include <stdarg.h>

namespace logif{

    static bool log_run = false;

#ifdef USE_LOG
    static const char log_name[] = "business_log";

    const static int kRotateLogFileSize = 1048576 * 5;
    const static int kRotateLogFileCount = 3;

    void rotateLog(const char *file_name, const int file_size, 
                const int file_count, const bool show_stdout){

        std::vector<spdlog::sink_ptr> sinks;  
        if(show_stdout){
            sinks.push_back(std::make_shared<spdlog::sinks::stdout_sink_mt>());  
        }
        sinks.push_back(std::make_shared<spdlog::sinks::rotating_file_sink_mt>(file_name, file_size, file_count));
        auto m_logger = std::make_shared<spdlog::logger>(log_name, begin(sinks), end(sinks));  
        m_logger->set_pattern("[%Y-%m-%d %H:%M:%S] %v");  
        spdlog::register_logger(m_logger);
    }

    bool makeDir(const char *dir_new){
        if (access(dir_new, 0) == -1){
            int flag=mkdir(dir_new, 0777);  
            if(flag != 0){
                printf("can't create logs dir\n");
                return false;
            }
        }
        return true;
    }

	void CreateLogger(const char *log_file, const bool show_stdout){

		std::string dir="./logs";  
    	if(!makeDir(dir.c_str())){
            dir = ".";
        }
    	std::string logfile = dir + "/" + log_file;
    	
        rotateLog(logfile.c_str(), kRotateLogFileSize, kRotateLogFileCount, show_stdout);
	     
        log_run = true;
    }

    void CreateLogger(const char *dir, const char *name, 
                        const bool show_stdout){

        std::string str_dir(dir);
        std::string file_name(name);

        // str_dir += "/" + file_name;

        // file_name += ".txt";

        // if(!makeDir(str_dir.c_str())){
        //     str_dir = dir;
        // }

        // std::string logfile(str_dir + "/" + file_name);

        std::string logfile(str_dir + "/" + file_name + ".txt");
        rotateLog(logfile.c_str(), kRotateLogFileSize, kRotateLogFileCount, show_stdout);

    }
	void DestroyLogger(){
		spdlog::drop_all();
	}

#endif

    void logIt(const char *fmt, ...){

        char temp[512];

        va_list args;       //定义一个va_list类型的变量，用来储存单个参数  
        va_start(args,fmt); //使args指向可变参数的第一个参数  
        vsnprintf(temp, 512, fmt, args);  //必须用vprintf等带V的  
        va_end(args);       //结束可变参数的获取  

        if(log_run){
#ifdef USE_LOG
            spdlog::get(log_name)->error(temp);
#endif
        }else{
            printf("%s\n", temp);
        }

    }

}