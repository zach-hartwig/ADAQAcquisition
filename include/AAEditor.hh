#ifndef __AAEditor_hh__
#define __AAEditor_hh__ 1

#include <TObject.h>
#include <TGTextEdit.h>
#include <TGButton.h>


class AAEditor : public TObject{
  
public:
  AAEditor(const TGWindow *, UInt_t, UInt_t);
  ~AAEditor();
  
  void LoadFile(const char *file);
  void LoadBuffer(const char *buffer);
  void AddBuffer(const char *buffer);
  
  TGTextEdit *GetEditor() { return Editor_TE; }
  TGWindow *GetEditorWindow(){return EditorWindow;}
  TGText *GetEditorText() { return EditorText_T; }
  
  void CloseWindow() { delete this; }
  void DoSave();    

  ClassDef(AAEditor, 1);
  
private:
  TGTransientFrame *EditorWindow;
  
  TGTextEdit *Editor_TE;
  
  TGText *EditorText_T;
  
  TGTextButton *Save_TB;
};

#endif
