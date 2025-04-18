/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2009
*
*****************************************************************************/

#include <fstream>
#include <list>
#include <string>
#include <cstdio>
#include <cassert>
#include <stdarg.h>
#include <sys/types.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <pthread.h>
#endif

#include "../Logger/Log.h"
#include "../Utility/FileUtils.h"
#include "../Utility/version.h"
#include "../Utility/Utils.h"
#include "../Utility/OptionIniFileMgr.h"
#include "../Host/Inc/RuntimeMemory.h"
#include "../Inc/CFlashToolConst.h"

using namespace std;

#define MONTH_STRING(p) (p->tm_mon + 1)
#define YEAR_STRING(p) (p->tm_year + 1900)

#ifdef _WIN32
#define DEFAULT_LOG_FILE "\\QT_FLASH_TOOL_V6.log"
static const char kSep= '\\';
#else
#define DEFAULT_LOG_FILE "/QT_FLASH_TOOL_V6.log"
static const char kSep= '/';
#endif

class LoggerImpl {
public:
    LoggerImpl(const string& loggerName, Logger::Level pcBaseLevel,
            Logger::Level curLevel);
    ~LoggerImpl();

    void AddLogHandle(const string& fileName, Logger::Level level);
    void RemovePCLogHandle();

    void SetCurrentLevel(Logger::Level level) {
        m_CurLevel = level;
}
    void SetPCLogBaseLevel(Logger::Level level) {
        m_PCLogBaseLevel = level;
    }
    bool PCLogFileSuppressed() const {
        return (m_CurLevel < m_PCLogBaseLevel);
    }
    bool ConsoleSuppressed() const
    {
        return m_CurLevel < m_ConsoleBaseLevel;
    }
    string ContextInfo() const;
    string GetTimestamp() const;

    template<typename T>
    void Put(T obj);

    static string GetLevelString(Logger::Level level);

private:
    void AddConsoleLogHandle();
    void AddLogHandle(ostream* stdStream, Logger::Level level);
    template<typename T>
    void PutToConsole(T obj);
    template<typename T>
    void PutToPCLogFile(T obj);
    void RemoveAllLogHandle();

private:
    struct LogHandle {
        string FileName;
        Logger::Level HandleLevel;
        ostream* OutStream;
        bool operator ==(const LogHandle &logHandle){
            return this->FileName == logHandle.FileName && this->HandleLevel == logHandle.HandleLevel
                    && this->OutStream == logHandle.OutStream;
        }
    };

    string m_LoggerName;
    Logger::Level m_PCLogBaseLevel;
    Logger::Level m_ConsoleBaseLevel;
    Logger::Level m_CurLevel;

    list<LogHandle> m_LogHandles;
};

string LoggerImpl::GetLevelString(Logger::Level level) {
    switch (level) {
    case Logger::Verbose:
        return "V";
    case Logger::Debug:
        return "D";
    case Logger::Info:
        return "I";
    case Logger::Warn:
        return "W";
    case Logger::Error:
        return "E";
    case Logger::Off:
        return "OFF";
    default:
        return "UNKNOWN";
    }
}

void LoggerImpl::AddConsoleLogHandle()
{
#ifdef _DEBUG
    AddLogHandle(&std::cout, Logger::Verbose);
#else
    AddLogHandle(&std::cout, Logger::Info);
#endif
}

LoggerImpl::LoggerImpl(const string &loggerName, Logger::Level pcBaseLevel,
        Logger::Level curLevel) :
    m_LoggerName(loggerName), m_PCLogBaseLevel(pcBaseLevel),
    m_ConsoleBaseLevel(Logger::Verbose), m_CurLevel(curLevel)
{
    AddConsoleLogHandle();
}

LoggerImpl::~LoggerImpl() {
    RemoveAllLogHandle();
    }

string LoggerImpl::ContextInfo() const {
    char infoBuffer[256] = { 0 };
#ifdef _WIN32
        DWORD pid = GetCurrentProcessId();
        DWORD tid = GetCurrentThreadId();
        SYSTEMTIME timestamp;
        GetLocalTime(&timestamp);
        sprintf(infoBuffer,
                        "%02d/%02d/%02d %02d:%02d:%02d.%03d %s[%lu][%lu][%s]: ",
                        timestamp.wMonth, timestamp.wDay, timestamp.wYear, timestamp.wHour,
                        timestamp.wMinute, timestamp.wSecond, timestamp.wMilliseconds,
                        m_LoggerName.c_str(), pid, tid, GetLevelString(m_CurLevel).c_str());
#else
    long pid = getpid();
    long tid = pthread_self();
// TODO Should add milli second.
    time_t cur_time;
    struct tm * pTime;
    time(&cur_time);
    pTime = localtime(&cur_time);
        sprintf(infoBuffer,
            "%02d/%02d/%02d %02d:%02d:%02d %s[%lu][%lu][%s]: ",
            MONTH_STRING(pTime), pTime->tm_mday, YEAR_STRING(pTime),
            pTime->tm_hour, pTime->tm_min, pTime->tm_sec,
            m_LoggerName.c_str(), pid, tid, GetLevelString(m_CurLevel).c_str());
#endif
    return string(infoBuffer);
}

string LoggerImpl::GetTimestamp() const {
        //char infoTimestamp[_MAX_FNAME] = { 0 };
    char infoTimestamp[260] = { 0 };
#ifdef _WIN32
    SYSTEMTIME timestamp;
    GetLocalTime(&timestamp);
    sprintf(infoTimestamp,
            "%02d-%02d-%02d-%02d-%02d-%02d",
            timestamp.wMonth, timestamp.wDay, timestamp.wYear, timestamp.wHour,
            timestamp.wMinute, timestamp.wSecond);
#else
    time_t cur_time;
    struct tm * pTime;
    time(&cur_time);
    pTime = localtime(&cur_time);
    sprintf(infoTimestamp,
            "%02d-%02d-%02d-%02d-%02d-%02d", MONTH_STRING(pTime),
            pTime->tm_mday, YEAR_STRING(pTime),
            pTime->tm_hour, pTime->tm_min, pTime->tm_sec);
#endif

    return string(infoTimestamp);
}

void LoggerImpl::AddLogHandle(const string& fileName, Logger::Level level) {
    assert(fileName != "");
    ofstream *fs = new ofstream(fileName.c_str(), std::ios_base::out
            | std::ios_base::app);
    if (fs != NULL && fs->is_open()) {
        LogHandle handle;
        handle.FileName = fileName;
        handle.HandleLevel = level;
        handle.OutStream = fs;
        m_LogHandles.push_back(handle);
    }
}

void LoggerImpl::AddLogHandle(ostream* stdStream, Logger::Level level) {
    assert(stdStream != NULL);
    LogHandle handle;
    handle.FileName = "";
    handle.HandleLevel = level;
    handle.OutStream = stdStream;
    m_LogHandles.push_back(handle);
}

void LoggerImpl::RemovePCLogHandle() {
    std::list<LogHandle> removeHandles;
    typedef std::list<LogHandle>::iterator LogHandleIterator;
    for (LogHandleIterator it = m_LogHandles.begin(); it
         != m_LogHandles.end(); ++it) {
        if (it->FileName != "") {
            removeHandles.push_back(*it);
        }
    }
    for (LogHandleIterator iter = removeHandles.begin(); iter != removeHandles.end(); ++iter)
    {
        if (iter->OutStream)
        {
            delete iter->OutStream;
        }
        m_LogHandles.remove(*iter);
    }
}

void LoggerImpl::RemoveAllLogHandle()
{
    for (list<LogHandle>::const_iterator it = m_LogHandles.begin(); it
         != m_LogHandles.end(); ++it) {
        if (it->FileName != "" && it->OutStream) {
            delete it->OutStream;
        }
    }
    m_LogHandles.clear();
}

template<typename T>
void LoggerImpl::Put(T obj) {
    PutToPCLogFile(obj);
    PutToConsole(obj);
}

template<typename T>
void LoggerImpl::PutToConsole(T obj)
{
    if (!ConsoleSuppressed())
    {
        for (list<LogHandle>::const_iterator it = m_LogHandles.begin();
             it != m_LogHandles.end(); ++it)
        {
            //the filename releated to console stream is empty
            if (m_CurLevel >= it->HandleLevel && it->FileName.empty() && it->OutStream)
            {
                *(it->OutStream) << obj;
                it->OutStream->flush();
            }
        }
    }
}

template<typename T>
void LoggerImpl::PutToPCLogFile(T obj)
{
    if (!PCLogFileSuppressed())
    {
        for (list<LogHandle>::const_iterator it = m_LogHandles.begin();
             it != m_LogHandles.end(); ++it)
        {
            //the filename releated to PC log file stream is not empty
            if (m_CurLevel >= it->HandleLevel && !it->FileName.empty() && it->OutStream)
            {
                *(it->OutStream) << obj;
                it->OutStream->flush();
            }
        }
    }
}

//Logger
Logger::Logger(const string& loggerName, Level baseLevel) {
    pimpl = new LoggerImpl(loggerName, baseLevel, Logger::Verbose);
}

Logger::~Logger() {
    if (pimpl){
        pimpl->Put("\nLogger deinited.");
        delete pimpl;
        pimpl = NULL;
    }
}

string Logger::ContextInfo() const {
    return pimpl->ContextInfo();
}

Logger& Logger::operator<<(Level level) {
    pimpl->SetCurrentLevel(level);
    return *this;
}

void Logger::SetPCLogBaseLevel(Logger::Level level) {
    pimpl->SetPCLogBaseLevel(level);
}

void Logger::AddLogHandle(const string& fileName, Logger::Level level) {
    pimpl->AddLogHandle(fileName, level);
}

void Logger::RemovePCLogHandle() {
    pimpl->RemovePCLogHandle();
}

CriticalSection &Logger::GetCriticalSection()
{
    return m_cs;
}

Logger& Logger::operator<<(ostream& (*pf)(ostream&)) {
    pimpl->Put(pf);
    return *this;
}
Logger& Logger::operator<<(ios_base& (*pf)(ios_base&)) {
    pimpl->Put(pf);
    return *this;
}
Logger& Logger::operator<<(ios& (*pf)(ios&)) {
    pimpl->Put(pf);
    return *this;
}
Logger& Logger::operator<<(bool val) {
    pimpl->Put(val);
    return *this;
}
Logger& Logger::operator<<(short val) {
    pimpl->Put(val);
    return *this;
}
Logger& Logger::operator<<(unsigned short val) {
    pimpl->Put(val);
    return *this;
}
Logger& Logger::operator<<(int val) {
    pimpl->Put(val);
    return *this;
}
Logger& Logger::operator<<(unsigned int val) {
    pimpl->Put(val);
    return *this;
}
Logger& Logger::operator<<(long val) {
    pimpl->Put(val);
    return *this;
}
Logger& Logger::operator<<(unsigned long val) {
    pimpl->Put(val);
    return *this;
}
Logger& Logger::operator<<(float val) {
    pimpl->Put(val);
    return *this;
}
Logger& Logger::operator<<(double val) {
    pimpl->Put(val);
    return *this;
}
Logger& Logger::operator<<(long double val) {
    pimpl->Put(val);
    return *this;
}
Logger& Logger::operator<<(const void* val) {
    pimpl->Put(val);
    return *this;
}
Logger& Logger::operator<<(streambuf* sb) {
    pimpl->Put(sb);
    return *this;
}
Logger& Logger::operator<<(const string& sb) {
    pimpl->Put(sb);
    return *this;
}
Logger& Logger::operator<<(char val) {
    pimpl->Put(val);
    return *this;
}
Logger& Logger::operator<<(const char* val) {
    pimpl->Put(val);
    return *this;
}
Logger& Logger::operator <<(const wchar_t* val) {
    pimpl->Put(val);
    return *this;
}

Logger& Logger::GetLogger(){
    static Logger mLogger("SPFlashToolV6");
    return mLogger;
}

void Logger::SetSPFlashToolLogFolder() {
    sp_flash_tool_log_path = OptionIniFileMgr::getInstance()->getLog_path().toStdString();
    if ( CREATE_DIR_FAIL == FileUtils::CheckDirectory(sp_flash_tool_log_path, true) ) {
        assert( 0 && "To create SP Flash Tool logs path fails!");
    }
}

std::string Logger::GetSPFlashToolLogFolder() {
    if (sp_flash_tool_log_path.empty()) {
        SetSPFlashToolLogFolder();
    }
    return sp_flash_tool_log_path;
}

std::string Logger::DumpFileWithTimeStamp() const {
#ifdef _WIN32
    return ( std::string("\\SP_FT_V6_Dump_") + pimpl->GetTimestamp() );
#else
    return ( std::string("/SP_FT_V6_Dump_") + pimpl->GetTimestamp() );
#endif
}

void Logger::SetSPFlashToolDumpFileFolder() {
    dump_files_path = GetSPFlashToolLogFolder() + DumpFileWithTimeStamp();
    if ( CREATE_DIR_FAIL == FileUtils::CheckDirectory(dump_files_path, true) ) {
        assert( 0 && "To create SP Flash Tool logs dump path fails!");
    }
}
std::string Logger::GetSPFlashToolDumpFileFolder() {
    if (this->dump_files_path.empty()) {
        SetSPFlashToolDumpFileFolder();
    }
    return dump_files_path;
}

void Logger::ClearLogPath()
{
    sp_flash_tool_log_path.clear();
    dump_files_path.clear();
}

void Logger::ResetLogPath(const string &new_log_path)
{
    this->ClearLogPath();
    sp_flash_tool_log_path = new_log_path;
}

void Logger::DebugOn() {
    SetPCLogBaseLevel(Logger::Verbose);

    AddLogHandle(GetSPFlashToolDumpFileFolder()+ DEFAULT_LOG_FILE, Logger::Verbose);

    CriticalSectionSentry lck(m_cs);
    Logger::GetLogger() << Logger::Debug << ToolInfo::ToString() << std::endl;
}

void Logger::DebugOff() {
    CriticalSectionSentry lck(m_cs);
    Logger::GetLogger() << Logger::Debug <<
            "================================================================="
            "================================================================="
            << std::endl << qPrintable(STRING_TOOL_NAME) <<
            "runtime trace is OFF"
            << std::endl <<
            "================================================================="
            "================================================================="
            << std::endl;
    SetPCLogBaseLevel(Logger::Off);
    RemovePCLogHandle();
    ClearLogPath();
}

void LogFunc::operator ()(const char * format,...)
{
    CriticalSectionSentry lck(Logger::GetLogger().GetCriticalSection());

    std::string log_str(format);
    va_list params;
    static char msg[5120] = {0};

    va_start( params, format );
    vsnprintf(msg, 5120, log_str.c_str(), params );
    va_end( params );

    std::string whole_name(m_file);
    std::string::size_type index = whole_name.find_last_of(kSep);
    if(index != whole_name.npos)
    {
        m_file += index+1;
    }

    Logger::GetLogger() << min(Logger::Debug, m_level);
    Logger::GetLogger() << Logger::GetLogger().ContextInfo()
                        << m_func << "(): ";

    Logger::GetLogger() << m_level;
    Logger::GetLogger() << msg;

    Logger::GetLogger() << min(Logger::Debug, m_level);
    Logger::GetLogger() << "("<<m_file << "," << m_line << ")";

    Logger::GetLogger() << m_level;
    Logger::GetLogger() << std::endl;
}
