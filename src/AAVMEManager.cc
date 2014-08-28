#include "AAVMEManager.hh"


AAVMEManager *AAVMEManager::TheVMEManager= 0;


AAVMEManager *AAVMEManager::GetInstance()
{ return TheVMEManager; }


AAVMEManager::AAVMEManager()
  : BREnable(true), DGEnable(true), HVEnable(true),
    DGAddress(0x00420000), HVAddress(0x42420000),
    VMEConnectionEstablished(false)
{;}


AAVMEManager::~AAVMEManager()
{;}
