#include "AATabSlots.hh"
#include "AAInterface.hh"

#include <iostream>
#include <sstream>


AATabSlots::AATabSlots(AAInterface *TI)
  : TheInterface(TI)
{;}


AATabSlots::~AATabSlots()
{;}


void AATabSlots::HandleConnectionTextButtons()
{
  TGTextButton *TextButton = (TGTextButton *) gTQSender;
  int TextButtonID = TextButton->WidgetId();
  /*
  
  enum{V1718, V1720, V6534};

  // Temporarily redirect the std::cout messages to a local buffer
  streambuf* StandardBuffer = cout.rdbuf();
  ostringstream NewBuffer;
  cout.rdbuf( NewBuffer.rdbuf() );
  
  switch(TextButtonID){
    
    // Connect AAInterface with VME boards
  case V1718Connect_TB_ID:
    
    // If no connection is presently established...
    if(!TheInterface->VMEConnectionEstablished){

      int V1720LinkOpen = -42;
      if(TheInterface->V1720Enable){
	TheInterface->V1720BoardAddress = TheInterface->BoardAddress_NEF[V1720]->GetHexNumber();
	V1720LinkOpen = TheInterface->DGManager->OpenLink(TheInterface->V1720BoardAddress);
	TheInterface->DGManager->Initialize();
      }
      
      int V1718LinkOpen = -42;

      if(TheInterface->V1718Enable){
	if(!TheInterface->V1720Enable and !TheInterface->V6534Enable){
	  V1718LinkOpen = TheInterface->BRManager->OpenLinkDirectly();
	}
	else if(TheInterface->V1720LinkOpen == 0)
	  V1718LinkOpen = TheInterface->BRManager->OpenLinkViaDigitizer(TheInterface->DGManager->GetBoardHandle(), true);
      }
      
      int V6534LinkOpen = -42;
      if(TheInterface->V6534Enable){
	TheInterface->V6534BoardAddress = TheInterface->BoardAddress_NEF[V6534]->GetHexNumber();
	V6534LinkOpen = TheInterface->HVManager->OpenLink(V6534BoardAddress);

	if(V6534LinkOpen == 0)
	  HVManager->SetToSafeState();
      }

      if(V1718LinkOpen == 0 or V1720LinkOpen==0 or V6534LinkOpen==0){
	TheInterface->V1718Connect_TB->SetBackgroundColor(ColorManager->Number2Pixel(8));
	TheInterface->V1718Connect_TB->SetText("Connected: click to disconnect");
	TheInterface->VMEConnectionEstablished = true;

	// Convert the new "std::cout" buffer into a TGText
	string InputString = NewBuffer.str();
	TGText *InputText = new TGText;
	InputText->LoadBuffer(InputString.c_str());


	// Update the connection TGTextView with the status messages
	ConnectionOutput_TV->AddText(InputText);
	ConnectionOutput_TV->ShowBottom();
	ConnectionOutput_TV->Update();
      }
      else
	TheInterface->V1718Connect_TB->SetBackgroundColor(ColorManager->Number2Pixel(2));
    }

    // If a connection is already established then terminate the connection
    else if(TheInterface->VMEConnectionEstablished){

      // Call the functions that accomplishes safe shutdown of the
      // V6534 and V1720 boards (powering down voltages, turning all
      // channels off, etc)
      TheInterface->HandleDisconnectAndTerminate(false);

      // Change the color and text of the button.
      TheInterface->V1718Connect_TB->SetBackgroundColor(ColorManager->Number2Pixel(2));
      TheInterface->V1718Connect_TB->SetText("Disconnected: click to connect");
      TheInterface->VMEConnectionEstablished = false;

      // Convert the new "std::cout" buffer into a TGText
      string InputString = NewBuffer.str();
      TGText *InputText = new TGText;
      InputText->LoadBuffer(InputString.c_str());
      
      // Update the connection TGTextView with the status messages
      ConnectionOutput_TV->AddText(InputText);
      ConnectionOutput_TV->ShowBottom();
      ConnectionOutput_TV->Update();
    }
    break;

    // Set the V6534Enable boolean that controls whether or not the
    // V6534 high voltage board should be presently used
  case V6534BoardEnable_TB_ID:
    if(TheInterface->BoardEnable_TB[V6534]->GetString() == "Board enabled"){
      TheInterface->BoardEnable_TB[V6534]->SetText("Board disabled");
      TheInterface->BoardEnable_TB[V6534]->SetBackgroundColor(ColorManager->Number2Pixel(2));
      TheInterface->V6534Enable = false;
    }
    else if(TheInterface->BoardEnable_TB[V6534]->GetString() == "Board disabled"){
      TheInterface->BoardEnable_TB[V6534]->SetText("Board enabled");
      TheInterface->BoardEnable_TB[V6534]->SetBackgroundColor(ColorManager->Number2Pixel(8));
      TheInterface->V6534Enable = true;
    }
    break;

    // Set the V1720Enable boolean that controls whether or not the
    // V1720 high voltage board should be presently used
  case V1720BoardEnable_TB_ID:
    if(TheInterface->BoardEnable_TB[V1720]->GetString() == "Board enabled"){
      TheInterface->BoardEnable_TB[V1720]->SetText("Board disabled");
      TheInterface->BoardEnable_TB[V1720]->SetBackgroundColor(ColorManager->Number2Pixel(2));
      TheInterface->V1720Enable = false;
    }
    else if(TheInterface->BoardEnable_TB[V1720]->GetString() == "Board disabled"){
      TheInterface->BoardEnable_TB[V1720]->SetText("Board enabled");
      TheInterface->BoardEnable_TB[V1720]->SetBackgroundColor(ColorManager->Number2Pixel(8));
      TheInterface->V1720Enable = true;
    }
    break;

  case V1718BoardEnable_TB_ID:
    if(TheInterface->BoardEnable_TB[V1718]->GetString() == "Board enabled"){
      TheInterface->BoardEnable_TB[V1718]->SetText("Board disabled");
      TheInterface->BoardEnable_TB[V1718]->SetBackgroundColor(ColorManager->Number2Pixel(2));
      TheInterface->V1718Enable = false;
    }
    else if(TheInterface->BoardEnable_TB[V1718]->GetString() == "Board disabled"){
      TheInterface->BoardEnable_TB[V1718]->SetText("Board enabled");
      TheInterface->BoardEnable_TB[V1718]->SetBackgroundColor(ColorManager->Number2Pixel(8));
      TheInterface->V1718Enable = true;
    }
    break;
    
  }

  // Redirect std::cout back to the normal terminal output
  cout.rdbuf(StandardBuffer);
  */
}
