//Sim03.cpp
//Simulates operating system running multiple processes with scheduling
//Updated to run I/O concurrently and use scheduling algorithms
//Input: Configuration file
//           specifies cycle time for different functions and system properties
//       Metadata file
//           list of operations to be simulated
//Output: Log of operations and timestamps for beginning and end of each
//            logged to monitor or file or both - specified in config
//by Austin Bachman
//November 25, 2016

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "SimpleQueue.cpp"
#include <fstream>
#include <stdlib.h>
#include <iomanip>
#include <pthread.h>
#include <semaphore.h>
#include "SimulatorFunctions.h"

using namespace std;

/* Global Constant Declarations //////////////////////////////////////////////*/

//Maximum number of threads
static const int MAX_THREADS = 1000;

//Scheduling algorithm identifiers
static const int RR = 0,
                 SRTF = 1,
                 SJF = 2;

//PCB states
static const int NEW = 0,
                 READY = 1,
                 RUNNING = 2,
                 WAITING = 3,
                 EXIT = 4; 
                 
//logTo identifiers
static const char L_FILE = 'f',
                  L_MONITOR = 'm',
                  L_BOTH = 'b'; 

/* Global Variable Declarations /////////////////////////////////////////////*/

//semaphores
sem_t monitors, hardDrives, printers, keyboards, writeOut;
bool *printerUsed, *hdUsed; //array holds which devices are being used

//threads
pthread_t io_thr[MAX_THREADS];
int threadCount = 0; //threads currently running
int threadIndex = -1; //index of thread being created in io_thr array

//total processes
int ProcessCount = 0;

//output stream
ostringstream output;

//Last memory location allocated
unsigned int memoryLocation = -1; //memory uninitialized

/* Structure Definitions /////////////////////////////////////////////////////*/

//holds configuration file data
struct ConfigType
{
    string mdf;    //metadata filepath
    string lgf;    //log filepath
    int quantum;
    int schedulingAlg; // RR = 0, SRTF = 1, SJF = 2
    int processor; 
    int monitor;
    int hardDrive;
    int printer;
    int keyboard;
    int memory;
    long int systemMemory;
    int blockSize;
    int printerCount;
    int hdCount;
    char logTo;     //L_FILE = 'f', L_MONITOR = 'm', L_BOTH = 'b'
};

//holds one metadata object
struct MetaDataType
{
    char code;
    string descriptor;
    int cycles;
    bool started;
};

//process control block
struct PCB 
{
    int state; //NEW = 0, READY = 1, RUNNING = 2, WAITING = 3, EXIT = 4
    int processNum; //process number
};

//holds an entire process
struct Process
{
    PCB control;
    int cacheCount; //number of caching operations completed
    SimpleQueue<MetaDataType> metaData; //queue of metadata for process
    MetaDataType current; //metaData currently in use
    int timeRemaining; //cycles left until complete
    bool completed; //if all functions are done
    int runningThreads; //number of threads running for this process
};

//holds arguments to ioThread() method
struct ThreadArg 
{
    ConfigType cfg;
    MetaDataType meta;
    struct timeval start;
    PCB control;
    bool* created;
    Process* running; //reference to calling process
};

/* Function Prototypes ///////////////////////////////////////////////////////*/

//takes as input ConfigType object
//and config file name
//reads configuration file
//into ConfigType object
void readConfig( char*, ConfigType& );

//takes metadata file name 
//and queue of metadata objects as input
//reads metadata input
//into metadatatype queue
void readInput( const string, vector<Process>& );
//takes as input one metadata object in string form
//parses descriptor from string
string parseData( const string& );

//takes as input one metadata object in string form
//parse cycle count from string and returns as int
int parseCycles( const string& );

//takes PCB, metadata object, config data object, initial time, and the
//  number of cycles already completed by the calling process as arguments
//runs a single metadata function until function terminates
//  or quantum limit is reached
//interrupts when limit is reached
//logs time at beginning, interruption, and end of each metadata function
//calculates time to wait for cycle, waits for simulated duration
//calls ioThread() for I/O operations
//  waits until thread has begun to continue
//returns the number of cycles that have been run
int run( PCB&, MetaDataType&, const ConfigType&, struct timeval, int );

//takes vector of all processes, config object, and start time as arguments
//runs a single process until either complete or quantum limit is reached
//interrupts when quantum limit is reached
void runProcess( Process&, const ConfigType&, struct timeval );

//takes vector of all processes as argument
//looks at the completion status of each process,
//if all are complete, returns true
bool checkCompleted( const vector<Process>& );

//takes vector of processes, scheduling algorithm identifier, and previous index
//  as arguments
//depending of the algorithm selected, returns the index of the next process
//  to run
int getSchedule( const vector<Process>&, int, int );

//takes as input a void* casted ThreadArg object
//gets input data from ThreadArg object
//runs a single I/O operation in a thread
//acquires semaphore for each I/O device, waits if 
//  all devices are in use, releases at end
//logs time at beginning, waits for simulated duration, logs time at end
//will interrupt currently running process to complete
void* ioThread( void* );

/* Function Implementations //////////////////////////////////////////////////*/

int main( int argc, char* argv[] )
{
    ConfigType config;
    SimpleQueue<MetaDataType> metaData;
    MetaDataType tmpData;
    vector<Process> program;
    struct timeval start;
    ofstream fout;
    int index, processIndex = -1;
    bool finished;

    /* Get Input */
    readConfig( argv[1], config );
    readInput( config.mdf, program );
    
    /* Initialize semaphores */
    sem_init( &monitors, 0, 1 ); //one monitor
    sem_init( &hardDrives, 0, config.hdCount );
    hdUsed = new bool[config.hdCount];
    for( index = 0; index < config.hdCount; index++ )
    {
        hdUsed[index] = false;
    }
    sem_init( &printers, 0, config.printerCount );
    printerUsed = new bool[config.printerCount];
    for( index = 0; index < config.printerCount; index++ )
    {
        printerUsed[index] = false;
    }
    sem_init( &keyboards, 0, 1 ); //one keyboard
    sem_init( &writeOut, 0, 1 ); //lock writing to output
    
    output << fixed << setprecision(6); //set decimal precision
    
    gettimeofday( &start, NULL ); //time at beginning of program


    /* Run Simulation */
    output << timePassed( start ) * .000001
           << " - Simulator program starting" << endl;
 
    while( !checkCompleted( program ) )
    {        
        processIndex = getSchedule( program, config.schedulingAlg, processIndex );
        
        sem_wait( &writeOut ); //wait for semaphore
        output << timePassed( start ) * .000001
               << " - OS: preparing process "
               << program[processIndex].control.processNum << endl;
        output << timePassed( start ) * .000001
               << " - OS: starting process "
               << program[processIndex].control.processNum << endl;
        sem_post( &writeOut ); //release semaphore
        
        //run a single process subject to quantum limit
        runProcess( program[processIndex], config, start ); 
        
        //check if a process has finished execution and output
        for( index = 0; index < ProcessCount; index++ )
        {
            if( program[index].control.state == EXIT && !program[index].completed
                        && program[index].runningThreads == 0 )
            {
                sem_wait( &writeOut ); //wait for semaphore
                output << timePassed( start ) * .000001
                       << " - OS: process " << program[index].control.processNum
                       << " completed" << endl;
                sem_post( &writeOut ); //release semaphore
                program[index].completed = true;
            }
        }
    }
    
    //wait for all threads to finish execution 
    //output when each process finishes   
    while( !finished )
    {
        finished = (threadCount == 0);
        
        for( index = 0; index < ProcessCount; index++ )
        {
            if( !program[index].completed && program[index].control.state == EXIT 
                        && program[index].runningThreads == 0 )
            {
                finished = false;
                
                sem_wait( &writeOut ); //wait for semaphore
                output << timePassed( start ) * .000001
                       << " - OS: process " << program[index].control.processNum
                       << " completed" << endl;
                sem_post( &writeOut ); //release semaphore
                program[index].completed = true;
            }
        }
    }
    
    output << timePassed( start ) * .000001
           << " - Simulator program ending" << endl;
    
    /* Output Log */
    if( config.logTo == L_MONITOR || config.logTo == L_BOTH )
    {
        cout << output.str();
    }
    
    if( config.logTo == L_FILE || config.logTo == L_BOTH )
    {
        fout.open( (config.lgf).c_str() );
        fout << output.str();
        fout.close();
    }

    /* Destroy semaphores */
    sem_destroy( &monitors );
    sem_destroy( &hardDrives );
    sem_destroy( &printers );
    sem_destroy( &keyboards );
    sem_destroy( &writeOut );
    
    delete hdUsed;
    delete printerUsed;
    
    return 0;
}

void readConfig( char* input, ConfigType& config )
{
    string buffer, tmp;
    ifstream fin;
    
    fin.open(input);
    
    if( fin ) //check if file opened
    {
        //get number data from config
        fin >> buffer;
        while( buffer.compare("Path:") != 0 )
        {
            fin >> buffer;
        }
        
        fin >> config.mdf;
        
        fin >> buffer;
        while( buffer.compare("Number:") != 0 )
        {
            fin >> buffer;
        }
        
        fin >> buffer;
        config.quantum = atoi(buffer.c_str());
        
        fin >> buffer;
        while( buffer.compare("Code:") != 0 )
        {
            fin >> buffer;
        }
        
        fin >> buffer;
        if( buffer.compare( "RR" ) == 0 )
        {
            config.schedulingAlg = 0;
        }
        else if( buffer.compare( "SRTF" ) == 0 )
        {
            config.schedulingAlg = 1;
        }
        else
        {
            config.schedulingAlg = 2;
        }
        
        fin >> buffer;
        while( buffer.compare("(msec):") != 0 )
        {
            fin >> buffer;
        }
        
        fin >> buffer;
        config.processor = atoi(buffer.c_str());

        fin >> buffer;
        while( buffer.compare("(msec):") != 0 )
        {
            fin >> buffer;
        }
        
        fin >> buffer;
        config.monitor = atoi(buffer.c_str());
        
        fin >> buffer;
        while( buffer.compare("(msec):") != 0 )
        {
            fin >> buffer;
        }
        
        fin >> buffer;
        config.hardDrive = atoi(buffer.c_str());    
        
        fin >> buffer;
        while( buffer.compare("(msec):") != 0 )
        {
            fin >> buffer;
        }
        
        fin >> buffer;
        config.printer = atoi(buffer.c_str());  
        
        fin >> buffer;
        while( buffer.compare("(msec):") != 0 )
        {
            fin >> buffer;
        }
        
        fin >> buffer;
        config.keyboard = atoi(buffer.c_str());  
        
        fin >> buffer;
        while( buffer.compare("(msec):") != 0 )
        {
            fin >> buffer;
        }
        
        fin >> buffer;
        config.memory = atoi(buffer.c_str());
        
        fin >> buffer;
        while( buffer.compare("memory") != 0 )
        {
            fin >> buffer;
        }
        
        fin >> buffer;
        fin >> tmp;
        if( buffer.compare("(kbytes):") == 0 )
        {
            config.systemMemory = atoi(tmp.c_str()); 
        } 
        else if( buffer.compare("(Mbytes):") == 0 )
        {
            config.systemMemory = atoi(tmp.c_str()) * 1000; 
        } 
        else if( buffer.compare("(Gbytes):") == 0 )
        {
            config.systemMemory = atoi(tmp.c_str()) * 1000000; 
        } 
        
        fin >> buffer;
        while( buffer.compare("(kbytes):") != 0 )
        {
            fin >> buffer;
        }
        
        fin >> buffer;
        config.blockSize = atoi(buffer.c_str());
        
        fin >> buffer;
        while( buffer.compare("quantity:") != 0 )
        {
            fin >> buffer;
        }
        
        fin >> buffer;
        config.printerCount = atoi(buffer.c_str());

        fin >> buffer;
        while( buffer.compare("quantity:") != 0 )
        {
            fin >> buffer;
        }
        
        fin >> buffer;
        config.hdCount = atoi(buffer.c_str());
                
        fin >> buffer;
        while( buffer.compare("to") != 0 )
        {
            fin >> buffer;
        }
        
        fin >> buffer;
        
        //log to both, monitor, or file
        if( buffer.compare("File") == 0 )
        {
            config.logTo = L_FILE;
        }
        else if( buffer.compare("Monitor") == 0 )
        {
            config.logTo = L_MONITOR;
        }
        else
        {
            config.logTo = L_BOTH;
        }
        
        fin >> buffer;
        while( buffer.compare("Path:") != 0 )
        {
            fin >> buffer;
        }
        
        fin >> config.lgf;  
    }
    
    else
    {
        output << "No configuration file found." << endl;
    }
    
    fin.close();
}
    
void readInput( const string mdf, vector<Process>& processList )
{
    ifstream fin;
    string buffer;
    char ctmp;
    Process* ptmp = new Process;
    MetaDataType mdtmp;
    
    fin.open( mdf.c_str() ); //open metadata file
    if( fin ) //check if file opened
    {
        fin >> buffer;
        
        while( buffer.compare("Code:") != 0 )
        {
            fin >> buffer;
        }

        buffer.clear();
        ptmp->timeRemaining = 0;
        while( ctmp != '.' && fin.get(ctmp) ) //while data is valid
        {
            if( ctmp != ';' && ctmp != ':' && ctmp != ','
                && ctmp != '.' && ctmp != '\n' )
            {
                buffer.push_back(ctmp); //push character to string
            }
            
            else if ( buffer.compare(" ") > 0 ) //not an empty string
            {
                if( buffer[0] == ' ' ) //remove leading space
                {
                    buffer.erase(0,1);
                }
                
                mdtmp.code = buffer[0];
                mdtmp.descriptor = parseData(buffer);
                mdtmp.cycles = parseCycles(buffer);
                mdtmp.started = false;
                
                ptmp->metaData.enqueue(mdtmp); //push data to queue
                ptmp->timeRemaining += mdtmp.cycles;
                buffer.clear(); //clear for next use
                
                if( mdtmp.descriptor.compare("end") == 0 && mdtmp.code == 'A' )
                {
                    ProcessCount++;
                    ptmp->control.processNum = ProcessCount;
                    ptmp->completed = false;
                    ptmp->cacheCount = 0;
                    ptmp->runningThreads = 0;
                    processList.push_back(*ptmp);
                    delete ptmp;
                    ptmp = new Process;
                    ptmp->timeRemaining = 0;
                }
            }
        }
    }
    
    else
    {
        output << "No metadata file found." << endl;
    }
    
    fin.close();
    delete ptmp;
    ptmp = NULL;
}

string parseData( const string& in )
{
    string out;
    int index = 0;
    
    if( in.compare("") > 0 ) //not empty
    {
        while( in[index] != '(' )
        {
            index++;
        }
        
        index++;
        
        while( in[index] != ')' )
        {
            out.push_back( in[index] );
            index++;
        }
    }
    
    return out;
} 

int parseCycles( const string& in )
{
    int index = 0;
    string cycleStr;
    
    if( in.compare("") > 0 ) //not empty
    {
        while( in[index] != ')' )
        {
            index++;
        }
        
        index++;
        
        while( in[index] != '\0' )
        {
            cycleStr.push_back( in[index] );
            index++;
        }
    }
    
    return atoi( cycleStr.c_str() );
}

int run( Process& running, const ConfigType& cfg, struct timeval start, int cycle )
{
    PCB* control = &(running.control);
    MetaDataType* runMeta = &(running.current);
    struct timeval current;
    float time;
    ThreadArg* args = new ThreadArg;
    bool threadCreated = false;

    if( runMeta->code == 'S' ) //Operating System
    {
        if( runMeta->descriptor.compare("start") == 0 )
        {                
            control->state = READY;    
        }
        
        else if( runMeta->descriptor.compare("end") == 0 )
        {
            control->state = EXIT;
        }
    }
        
    else if( runMeta->code == 'A' ) //Program Application
    {
        if( runMeta->descriptor.compare("start") == 0 )
        {
            control->state = RUNNING;
        }
        else if( runMeta->descriptor.compare("end") == 0 )
        {
            control->state = EXIT;
        }            
    }
    
    while( runMeta->cycles > 0 && cycle < cfg.quantum && !threadCreated )
    {        
        time = timePassed( start ) * .000001; //convert to seconds
        gettimeofday( &current, NULL ); //get current time
        
        if( runMeta->code == 'P' ) //Process
        {   
            if( !runMeta->started )
            {
                sem_wait( &writeOut ); //wait for semaphore
                output << time
                       << " - Process " << control->processNum
                       << " start processing action" << endl;
                sem_post( &writeOut ); //release semaphore
                runMeta->started = true;
            }
            
            while( timePassed(current) < ( cfg.processor * 1000 ) ); //wait one cycle
            runMeta->cycles--;
            
            if( runMeta->cycles == 0 )
            {
                time = timePassed( start ) * .000001;
                sem_wait( &writeOut ); //wait for semaphore
                output << time
                       << " - Process " << control->processNum
                       << " end processing action" << endl;
                sem_post( &writeOut ); //release for semaphore
            }
            else if( cycle == cfg.quantum - 1 )
            {
                time = timePassed( start ) * .000001;
                sem_wait( &writeOut ); //wait for semaphore
                output << time
                       << " - Process " << control->processNum
                       << " interrupt processing action" << endl;
                sem_post( &writeOut ); //release semaphore
            }
        }
        
        else if( runMeta->code == 'M' ) //Memory
        {
            if( runMeta->descriptor.compare("allocate") == 0 )
            {
                if( !runMeta->started )
                {
                    sem_wait( &writeOut ); //wait for semaphore
                    output << time << " - Process " << control->processNum
                           << " allocating memory" << endl;
                    sem_post( &writeOut ); //release semaphore
                    runMeta->started = true;
                }
                
                while( timePassed(current) < ( cfg.memory * 1000 ) ); //wait one cycle
                runMeta->cycles--;
                
                if( runMeta->cycles == 0 )
                {
                    memoryLocation = AllocateMemory( cfg.systemMemory, cfg.blockSize, memoryLocation );
                    time = timePassed( start ) * .000001;

                    sem_wait( &writeOut ); //wait for semaphore
                    output << time
                           << " - Process " << control->processNum
                           << " memory allocated at "
                           << hex << "0x" << setw(8) << setfill('0')
                           << memoryLocation << endl;
                    sem_post( &writeOut ); //release semaphore
                }
                else if( cycle == cfg.quantum - 1 )
                {
                    sem_wait( &writeOut ); //wait for semaphore
                    output << time << " - Process " << control->processNum
                           << " interrupt memory allocation" << endl;
                    sem_post( &writeOut ); //release semaphore
                }
            }
            
            if( runMeta->descriptor.compare("cache") == 0 )
            {
                if( !runMeta->started )
                {
                    sem_wait( &writeOut ); //wait for semaphore
                    output << time
                           << " - Process " << control->processNum
                           << " start memory caching" << endl;
                    sem_post( &writeOut ); //release semaphore
                    runMeta->started = true;
                }
                
                while( timePassed(current) < ( cfg.memory * 1000 ) ); //wait one cycle
                runMeta->cycles--;
                       
                if( runMeta->cycles == 0 )
                {
                    time = timePassed( start ) * .000001;
                    sem_wait( &writeOut ); //wait for semaphore
                    output << time
                           << " - Process " << control->processNum
                           << " end memory caching" << endl;
                    sem_post( &writeOut ); //release semaphore
                    running.cacheCount++; //increment number of cache operations
                    
                }
                else if( cycle == cfg.quantum - 1 )
                {
                    sem_wait( &writeOut ); //wait for semaphore
                    output << time
                           << " - Process " << control->processNum
                           << " interrupt memory caching" << endl;
                    sem_post( &writeOut ); //release semaphore
                }
            }
        }
        
        else if( runMeta->code == 'I' || runMeta->code == 'O' ) //I/O in thread
        {
            args->cfg = cfg;
            args->meta = *runMeta;
            args->start = start;
            args->control = *control;
            args->running = &running;
            args->created = &threadCreated; //set in thread
            
            threadCount++;
            threadIndex++;
            running.runningThreads++;
            pthread_create( &io_thr[threadIndex], NULL, ioThread, (void*)args ); //create
            while( !threadCreated ); //wait for thread to be created
            cycle = runMeta->cycles;
        }
        
        cycle++;
    }
    
    delete args;
    args = NULL;
    
    return cycle; //number of cycles completed
}

void runProcess( Process& running, const ConfigType& cfg, struct timeval start )
{
    bool dequeued = true;
    int cyclesRun = 0;
    int updateCycles; //if a cache operation has occurred, change run cycle num
    
    while( cyclesRun < cfg.quantum && running.control.state != EXIT )
    {
        if( running.current.cycles <= 0 || running.current.code == 'I'
                                || running.current.code == 'O' )
        {
            dequeued = running.metaData.dequeue( running.current ); //get process to run
            //update cycles for processor
            //if caching operation has occurred
            if( running.current.descriptor.compare("run") == 0 )
            {
                updateCycles = max( 1, running.current.cycles - 2 * running.cacheCount );
                //update time left in process
                running.timeRemaining -= ( running.current.cycles - updateCycles );
                running.current.cycles = updateCycles;
            }    
        }
    
        if( dequeued )
        {
            cyclesRun = run( running, cfg, start, cyclesRun );   
        }
    }
    
    running.timeRemaining -= cyclesRun; 
}

bool checkCompleted( const vector<Process>& program )
{
    int index;
    bool completed = true;
    
    for( index = 0; index < ProcessCount; index++ )
    {
        completed &= ( program[index].control.state == EXIT );
    }

    return completed;
}

int getSchedule( const vector<Process>& program, int algorithmID, int prevIndex )
{
    int index = 0, retIndex;
    int minTimeRemaining = 100000;
    
    if( algorithmID == RR )
    {
        index = ( prevIndex + 1 ) % ProcessCount;
        while( program[index].control.state == EXIT )
        {
            index = ( index + 1 ) % ProcessCount;
        }
        
        return index;
    }
    else //SJF or SRTF
    {
        for( index = 0; index < ProcessCount; index++ )
        {
            if( program[index].control.state != EXIT )
            {
                if( program[index].timeRemaining < minTimeRemaining )
                {
                    minTimeRemaining = program[index].timeRemaining;
                    retIndex = index;
                }
            }
        }
        
        return retIndex;
    }
}

void* ioThread( void* arg )
{
    bool* created = ((ThreadArg*)arg)->created;
    ConfigType cfg = ((ThreadArg*)arg)->cfg;
    MetaDataType meta = ((ThreadArg*)arg)->meta;
    struct timeval start = ((ThreadArg*)arg)->start, current;
    Process* running = ((ThreadArg*)arg)->running;
    PCB control = ((ThreadArg*)arg)->control;
    float time;
    int semNum;  

    if( meta.descriptor.compare( "hard drive" ) == 0 ) //hard drive operation
    {   
        sem_wait( &hardDrives ); //wait for semaphore
        for(semNum = 0; semNum < cfg.hdCount; semNum++)
        {
            if( hdUsed[semNum] == false )
            {
                hdUsed[semNum] = true;
                break;
            }
        }
        gettimeofday( &current, NULL );
        time = timePassed( start ) * .000001; //convert to seconds

        if( meta.code == 'I' ) //hard drive input
        {
            sem_wait( &writeOut ); //wait for semaphore
            output << time
                   << " - Process " << control.processNum
                   << " start hard drive input on HDD " 
                   << semNum << endl;  
            sem_post( &writeOut ); //release semaphore 
            *created = true;
            while( timePassed(current)
                   < ( cfg.hardDrive * meta.cycles * 1000 ) ); //wait
            
            time = timePassed( start ) * .000001;

            sem_wait( &writeOut ); //wait for semaphore
            output << time
                   << " - Process " << control.processNum
                   << " end hard drive input on HDD "
                   << semNum << endl;
            sem_post( &writeOut ); //release semaphore
        }
        else if( meta.code == 'O' ) //hard drive output
        {
            sem_wait( &writeOut ); //wait for semaphore
            output << time
                   << " - Process " << control.processNum
                   << " start hard drive output on HDD "
                   << semNum << endl;
            sem_post( &writeOut ); //release semaphore
            *created = true;
            while( timePassed(current)
                   < ( cfg.hardDrive * meta.cycles * 1000 ) ); //wait
            
            time = timePassed( start ) * .000001;

            sem_wait( &writeOut ); //wait for semaphore
            output << time
                   << " - Process " << control.processNum
                   << " end hard drive output on HDD "
                   << semNum << endl;
            sem_post( &writeOut ); //release semaphore
        }

        hdUsed[semNum] = false;
        sem_post( &hardDrives ); //release semaphore
    }
        
    else if( meta.descriptor.compare( "keyboard" ) == 0 ) //keyboard input
    {   
        sem_wait( &keyboards ); //get semaphore
        gettimeofday( &current, NULL );
        time = timePassed( start ) * .000001; //convert to seconds

        sem_wait( &writeOut ); //wait for semaphore
        output << time
               << " - Process " << control.processNum
               << " start keyboard input" << endl;
        sem_post( &writeOut ); //release semaphore
        *created = true;
        while( timePassed(current)
               < ( cfg.keyboard * meta.cycles * 1000 ) ); //wait
        
        time = timePassed( start ) * .000001;

        sem_wait( &writeOut ); //wait for semaphore
        output << time
               << " - Process " << control.processNum
               << " end keyboard input" << endl;
        sem_post( &writeOut ); //release semaphore
        
        sem_post( &keyboards ); //release semaphore
    }
    
    else if( meta.descriptor.compare( "monitor" ) == 0 ) //monitor output
    {
        sem_wait( &monitors ); //get semaphore
        gettimeofday( &current, NULL );
        time = timePassed( start ) * .000001; //convert to seconds
    
        sem_wait( &writeOut ); //wait for semaphore
        output << time
               << " - Process " << control.processNum
               << " start monitor output" << endl;
        sem_post( &writeOut ); //release semaphore
        *created = true;
        while( timePassed(current)
               < ( cfg.monitor * meta.cycles * 1000 ) ); //wait
        
        time = timePassed( start ) * .000001;
   
        sem_wait( &writeOut ); //wait for semaphore
        output << time
               << " - Process " << control.processNum
               << " end monitor output" << endl;
        sem_post( &writeOut ); //release semaphore
        
        sem_post( &monitors ); //release semaphore
    }       
    
    else if( meta.descriptor.compare( "printer" ) == 0 ) //printer output
    {
        sem_wait( &printers ); //get semaphore
        for(semNum = 0; semNum < cfg.printerCount; semNum++)
        {
            if( printerUsed[semNum] == false )
            {
                printerUsed[semNum] = true;
                break;
            }
        }
        gettimeofday( &current, NULL );
        time = timePassed( start ) * .000001; //convert to seconds
               
        sem_wait( &writeOut ); //wait for semaphore
        output << time
               << " - Process " << control.processNum
               << " start printer output on PRNTR " 
               << semNum << endl;
        sem_post( &writeOut ); //release semaphore
        *created = true;
        while( timePassed(current)
               < ( cfg.printer * meta.cycles * 1000 ) ); //wait
        
        time = timePassed( start ) * .000001;

        sem_wait( &writeOut ); //wait for semaphore
        output << time
               << " - Process " << control.processNum
               << " end printer output on PRNTR "
               << semNum << endl;
        sem_post( &writeOut ); //release semaphore
        
        printerUsed[semNum] = false;
        sem_post( &printers ); //release semaphore  
    }
    threadCount--;
    running->runningThreads--;
    pthread_exit(NULL);
}

