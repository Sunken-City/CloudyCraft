#pragma once

class TextBox;

class MemoryOutputWindow
{
public:
    //CONSTRUCTORS/////////////////////////////////////////////////////////////////////
    MemoryOutputWindow();
    ~MemoryOutputWindow();

    //FUNCTIONS/////////////////////////////////////////////////////////////////////
    void Update(float deltaSeconds);
    void Render() const;
    inline void Hide() { m_isHidden = true; };
    inline void Show() { m_isHidden = false; };

    static MemoryOutputWindow* instance;

    //MEMBER VARIABLES//////////////////////////////////////////////////////////////////////////
    TextBox* m_outputWindow;
    bool m_isHidden;
};