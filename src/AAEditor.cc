#include "AAEditor.hh"

#include <iostream>
using namespace std;


AAEditor::AAEditor(const TGWindow *MainWindow, UInt_t Width, UInt_t Height)
{
  EditorWindow = new TGTransientFrame(gClient->GetRoot(), 
				      MainWindow, 
				      Width, 
				      Height);
  EditorWindow->Connect("CloseWindow()", "AAEditor", this, "CloseWindow()");
  EditorWindow->DontCallClose();
  EditorWindow->SetCleanup(kDeepCleanup);
  
  EditorWindow->AddFrame(Editor_TE = new TGTextEdit(EditorWindow, 
						    Width, 
						    Height, 
						    kSunkenFrame | kDoubleBorder),
			 new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 3,3,3,3));
  Editor_TE->Connect("Closed()", "AAEditor", this, "CloseWindow()");
  

  Pixel_t White, DodgerBlue;
  gClient->GetColorByName("#1e90ff", DodgerBlue);
  gClient->GetColorByName("#ffffff", White);

  Editor_TE->SetSelectFore(White);
  Editor_TE->SetSelectBack(DodgerBlue);
  
  EditorWindow->AddFrame(Save_TB = new TGTextButton(EditorWindow, "  &Save  "),
			 new TGLayoutHints(kLHintsBottom | kLHintsCenterX, 0, 0, 5, 5));
  Save_TB->Connect("Clicked()", "AAEditor", this, "DoSave()");
  
  
  EditorWindow->SetWindowName("ADAQ File Comment Editor");
  EditorWindow->MapSubwindows();
  EditorWindow->Resize();
  EditorWindow->MapWindow();

  // editor covers right half of parent window
  EditorWindow->CenterOnParent(kTRUE, TGTransientFrame::kRight);
}


AAEditor::~AAEditor()
{ EditorWindow->DeleteWindow(); }


void AAEditor::LoadBuffer(const char *buffer)
{ Editor_TE->LoadBuffer(buffer); }


void AAEditor::LoadFile(const char *file)
{ Editor_TE->LoadFile(file); }


void AAEditor::AddBuffer(const  char *buffer)
{
  TGText Text;
  Text.LoadBuffer(buffer);
  Editor_TE->AddText(&Text);
}


void AAEditor::DoSave()
{ 
  EditorText_T = Editor_TE->GetText();
  CloseWindow();
}

