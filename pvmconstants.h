#include <iostream.h>
#ifndef pvmconstants_h
#define pvmconstants_h

/* AJ 14.02.02
 * Class PVMConstants contains information about different tags which
 * can be used while sending/receiving data using PVM. It also contains
 * information about which dataencoding to use while sending/receiving
 * data using PVM. Dataencoding can have values 0, 1 or 2 and can be
 * set and accessed. If dataencode is not set then dataencoding equals
 * to 0. But the different tags can not be changed. The class PVMConstants
 * only provides functions for accessing different tags */

class PVMConstants {
public:
  PVMConstants();
  ~PVMConstants();
  const void setDataEncode(const int dc);
  const int getStopTag() const;
  const int getStartTag() const;
  const int getMasterSendVarTag() const;
  const int getMasterSendStringTag() const;
  const int getMasterSendBoundTag() const;
  const int getMasterReceiveDataTag() const;
  const int getDiedTag() const;
  const int getDataEncode() const;
  const int getAddHostTag()const;
  const int getHostSuspendTag() const;
  const int getHostResumeTag() const;
  const int getHostDeleteTag() const;
  const int getTaskExitTag() const;
private:
  // tags for messages send/received using pvm
  int endType;                // indicate halting of netcommunication
  int startType;              // indicate starting of netcommunication
  int taskDied;               // indicate that process had died
  int masterSendNetDataVar;   // indicate sending of netvariables by master
  int masterSendString;       // indicate sending of string by master
  int masterSendBound;        // indicate sending of bounds vector by master
  int masterReceiveNetDataResult; // indicate receive netdataresult from slave
  int dataencode;
  //Possibilities for dataencode are: (default value 0)
  //dataencode = 0 then use XDR encoding for receiving/sending data
  //dataencode = 1 then no encoding of data
  //dataencode = 2 then data is left in place when sending/reciving
  //For condor
  int addHost;     // indicate that host has been added to virtual machine
  int hostSuspend; // indicate that host has been suspended
  int hostResume;  // indicate that host has resumed
  int hostDelete;  // indicate that host has been deleted
  int taskExit;    // indicate that host task has exited
};

#endif
