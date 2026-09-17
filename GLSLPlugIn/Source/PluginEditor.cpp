/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/
#include "StaticValues.h"

#include "PluginProcessor.h"
#include "PluginEditor.h"

#include <iostream>

//==============================================================================
GlslplugInAudioProcessorEditor::GlslplugInAudioProcessorEditor (GlslplugInAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p), fragmentEditorComp (fragmentDocument, nullptr), forwardFFT (fftOrder), fifoIndex (0), nextFFTBlockReady (false)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (wndFullSizeW, wndFullSizeH);
    getTopLevelComponent()->addKeyListener (this);

    m_GLSLCompo.setStatusLabelPtr (&m_statusLabel);
    m_GLSLCompo.setFragmentDocPtr (&fragmentDocument);
    addAndMakeVisible (m_GLSLCompo);

    Colour editorBackground (Colours::darkgrey);
    Colour editorForeground (Colours::white);

    Colour windowBackground (Colours::darkcyan);
    Colour windowForeground (Colours::white);

    fragmentEditorComp.setColour (CodeEditorComponent::backgroundColourId, editorBackground);
    fragmentEditorComp.setColour (CodeEditorComponent::defaultTextColourId, editorForeground);
    fragmentDocument.addListener (this);
    addAndMakeVisible (fragmentEditorComp);

    m_statusLabel.setJustificationType (Justification::topLeft);
    m_statusLabel.setColour (Label::backgroundColourId, windowBackground);
    m_statusLabel.setColour (Label::textColourId, windowForeground);
    m_statusLabel.setFont (Font (14.0f));
    addAndMakeVisible (m_statusLabel);

    m_SyncModeSwitch.setToggleState (false, dontSendNotification);
    m_SyncModeSwitch.setColour (Label::backgroundColourId, windowBackground);
    m_SyncModeSwitch.setColour (Label::textColourId, windowForeground);
    m_SyncModeSwitch.setButtonText ("Auto Sync");
    m_SyncModeSwitch.setName ("AUTO_MODE");
    m_SyncModeSwitch.addListener (this);
    addAndMakeVisible (m_SyncModeSwitch);

    m_SyncButton.setColour (Label::backgroundColourId, Colours::darkmagenta);
    m_SyncButton.setColour (Label::textColourId, Colours::white);
    m_SyncButton.setButtonText ("Sync Player");
    m_SyncButton.setName ("SYNC");
    m_SyncButton.addListener (this);
    addAndMakeVisible (m_SyncButton);

    m_PlayWndButton.setColour (Label::backgroundColourId, Colours::darkmagenta);
    m_PlayWndButton.setColour (Label::textColourId, Colours::white);
    m_PlayWndButton.setButtonText ("Player Window");
    m_PlayWndButton.setName ("PLAY_WND");
    m_PlayWndButton.addListener (this);
    addAndMakeVisible (m_PlayWndButton);

    fragmentDocument.replaceAllContent (GLSLComponent::defaultFragmentShader);

    checkForCommandLineInput();

    startTimer (shaderLinkDelay);
}

void GlslplugInAudioProcessorEditor::checkForCommandLineInput()
{
    const auto args = JUCEApplicationBase::getCommandLineParameterArray();
    String inputPath;

    for (int i = 0; i < args.size(); ++i)
    {
        if (args[i] == "-i" || args[i] == "--input")
        {
            if (i + 1 < args.size())
                inputPath = args[i + 1];

            break;
        }
    }

    if (inputPath.isEmpty())
        return;

    File shaderFile (inputPath);

    if (! shaderFile.existsAsFile())
    {
        std::cerr << "error: cannot read shader file: " << inputPath << std::endl;

        if (auto* app = JUCEApplicationBase::getInstance())
        {
            app->setApplicationReturnValue (1);
            JUCEApplicationBase::quit();
        }

        return;
    }

    commandLineInputSource = shaderFile.loadFileAsString();
    commandLineInputPending = true;
}

GlslplugInAudioProcessorEditor::~GlslplugInAudioProcessorEditor()
{
    stopTimer();
}

//==============================================================================
void GlslplugInAudioProcessorEditor::paint (Graphics& g)
{
    g.fillAll (Colours::darkcyan);
}

void GlslplugInAudioProcessorEditor::resized()
{
    Rectangle<int> area (getLocalBounds().reduced (4));
    Rectangle<int> bottom (area.removeFromBottom (75));

    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    int glSide = wndFullSizeH;
    int glBottom = wndFullSizeH;
    m_GLSLCompo.setBounds (0, 0, glSide, glBottom);

    int editorBottom = wndFullSizeH - 80;
    int editorSide = wndFullSizeW - wndFullSizeH;

    switch (m_guiState)
    {
        case GUIState::Default:
            fragmentEditorComp.setBounds (glSide, 0, editorSide, editorBottom);
            m_statusLabel.setBounds (glSide, editorBottom, editorSide, 60);
            m_SyncModeSwitch.setBounds (glSide, editorBottom + 60, 108, 20);
            m_SyncButton.setBounds (glSide + 108, editorBottom + 60, 170, 20);
            m_PlayWndButton.setBounds (glSide + 278, editorBottom + 60, 170, 20);
            m_GLSLCompo.setVisible (true);
            fragmentEditorComp.setVisible (true);
            m_SyncModeSwitch.setVisible (true);
            m_SyncButton.setVisible (true);
            m_PlayWndButton.setVisible (true);
            m_statusLabel.setVisible (true);
            this->setSize (wndFullSizeW, wndFullSizeH);
            break;

        case GUIState::EditorOnly:
            fragmentEditorComp.setBounds (0, 0, wndFullSizeW, editorBottom);
            m_statusLabel.setBounds (0, editorBottom, wndFullSizeW, 60);
            m_SyncModeSwitch.setBounds (glSide, editorBottom + 60, 108, 20);
            m_SyncButton.setBounds (glSide + 108, editorBottom + 60, 170, 20);
            m_PlayWndButton.setBounds (glSide + 278, editorBottom + 60, 170, 20);
            m_GLSLCompo.setVisible (false);
            fragmentEditorComp.setVisible (true);
            m_SyncModeSwitch.setVisible (true);
            m_SyncButton.setVisible (true);
            m_PlayWndButton.setVisible (true);
            m_statusLabel.setVisible (true);
            this->setSize (wndFullSizeW, wndFullSizeH);
            break;

        case GUIState::PreviewOnly:
            fragmentEditorComp.setBounds (glSide, 0, editorSide, editorBottom);
            m_statusLabel.setBounds (glSide, editorBottom, editorSide, 60);
            m_SyncModeSwitch.setBounds (glSide, editorBottom + 60, 108, 20);
            m_SyncButton.setBounds (glSide + 108, editorBottom + 60, 170, 20);
            m_PlayWndButton.setBounds (glSide + 278, editorBottom + 60, 170, 20);
            m_GLSLCompo.setVisible (true);
            fragmentEditorComp.setVisible (false);
            m_SyncModeSwitch.setVisible (false);
            m_SyncButton.setVisible (false);
            m_PlayWndButton.setVisible (false);
            m_statusLabel.setVisible (false);
            this->setSize (glSide, glBottom);
            break;
    }
}

//==============================================================================
void GlslplugInAudioProcessorEditor::timerCallback()
{
    if (commandLineInputPending && m_GLSLCompo.isInitialised)
    {
        commandLineInputPending = false;
        commandLineInputAwaitingResult = true;
        fragmentDocument.replaceAllContent (commandLineInputSource);
    }

    if (isNeedShaderCompile)
    {
        isNeedShaderCompile = false;

        stopTimer();

        StaticValues::setShaderCache (fragmentDocument.getAllContent());

        if (m_GLSLCompo.isInitialised)
            m_GLSLCompo.setShaderProgramFragment (StaticValues::getShaderCache());

        if (isShaderSyncAuto)
            setShaderSync();

        startTimer (20);
    }

    if (commandLineInputAwaitingResult
        && m_GLSLCompo.isInitialised
        && ! isNeedShaderCompile
        && ! m_GLSLCompo.isShaderCompileReady)
    {
        commandLineInputAwaitingResult = false;

        const bool success = m_GLSLCompo.isShaderCompileSuccess;
        std::cout << (success ? "OK: " : "ERROR: ") << m_statusLabel.getText() << std::endl;

        if (auto* app = JUCEApplicationBase::getInstance())
        {
            app->setApplicationReturnValue (success ? 0 : 1);
            JUCEApplicationBase::quit();
        }
    }

    // MIDI CC
    if (! m_midiCCqueue.empty())
    {
        sendMidiCCValue();
    }

    // Wave
    if (nextWaveBlockReady)
    {
        sendNextWave();
        nextWaveBlockReady = false;
    }

    // FFT
    if (nextFFTBlockReady)
    {
        sendNextSpectrum();
        nextFFTBlockReady = false;
    }
}

void GlslplugInAudioProcessorEditor::codeDocumentTextInserted (const String& /*newText*/, int /*insertIndex*/)
{
    startTimer (shaderLinkDelay);
    isNeedShaderCompile = true;

    // CodeEditorComponent's incremental repaint can leave stale glyphs on screen
    // when a line shrinks (e.g. after a bulk replaceAllContent with shorter text),
    // since it only invalidates the region covered by the new content. Force a
    // full repaint so nothing from the previous content lingers.
    fragmentEditorComp.repaint();
}

void GlslplugInAudioProcessorEditor::codeDocumentTextDeleted (int /*startIndex*/, int /*endIndex*/)
{
    startTimer (shaderLinkDelay);
    isNeedShaderCompile = true;
    fragmentEditorComp.repaint();
}

void GlslplugInAudioProcessorEditor::setMidiCCValue (juce::MidiMessage midiCC)
{
    m_midiCCqueue.push (midiCC);
}

void GlslplugInAudioProcessorEditor::sendMidiCCValue()
{
    while (! m_midiCCqueue.empty())
    {
        juce::MidiMessage midiCC = m_midiCCqueue.front();
        m_midiCCqueue.pop();
        if (m_GLSLCompo.isInitialised)
            m_GLSLCompo.setMidiCCValue (midiCC.getControllerNumber(), midiCC.getControllerValue());
    }
}

void GlslplugInAudioProcessorEditor::pushNextSampleIntoFifo (float sample) noexcept
{
    // if the fifo contains enough data, set a flag to say
    // that the next line should now be rendered..
    if (fifoIndex == fftSize)
    {
        if (! nextWaveBlockReady)
        {
            zeromem (waveData, sizeof (waveData));
            memcpy (waveData, fifo, sizeof (fifo));
            nextWaveBlockReady = true;
        }

        if (! nextFFTBlockReady)
        {
            zeromem (fftData, sizeof (fftData));
            memcpy (fftData, fifo, sizeof (fifo));
            nextFFTBlockReady = true;
        }

        fifoIndex = 0;
    }

    fifo[fifoIndex++] = sample;
}

void GlslplugInAudioProcessorEditor::setShaderSync()
{
    StaticValues::setNeedShaderSync (true);
}

void GlslplugInAudioProcessorEditor::sendNextSpectrum()
{
    // then render our FFT data..
    forwardFFT.performFrequencyOnlyForwardTransform (fftData);

    // find the range of values produced, so we can scale our rendering to
    // show up the detail clearly
    for (int i = 0; i < fftSize; i++)
    {
        auto spectrumVal = fftData[i];
        if (m_GLSLCompo.isInitialised)
            m_GLSLCompo.setSpectrumValue (i, spectrumVal * spectrumVal);
    }
}

void GlslplugInAudioProcessorEditor::sendNextWave()
{
    for (int i = 0; i < fftSize; i++)
    {
        if (m_GLSLCompo.isInitialised)
            m_GLSLCompo.setWaveValue (i, waveData[i]);
    }
}

bool GlslplugInAudioProcessorEditor::keyPressed (const KeyPress& key, Component* originatingComponent)
{
    //m_statusLabel.setText(m_statusLabel.getText() + "keyCode:" + String(key.getKeyCode()) + "/keyChar:" + String(key.getTextCharacter()), dontSendNotification);

    if (key.getModifiers() == ModifierKeys::ctrlModifier)
    {
        if (key.getKeyCode() == 75) // "k"
        {
            switch (m_guiState)
            {
                case GUIState::Default:
                    m_guiState = GUIState::PreviewOnly;
                    break;
                case GUIState::EditorOnly:
                    break;
                case GUIState::PreviewOnly:
                    m_guiState = GUIState::Default;
                    break;
            }
            this->resized();
        }
        if (key.getKeyCode() == 80) // "p"
        {
            switch (m_guiState)
            {
                case GUIState::Default:
                    m_guiState = GUIState::EditorOnly;
                    break;
                case GUIState::EditorOnly:
                    m_guiState = GUIState::Default;
                    break;
                case GUIState::PreviewOnly:
                    break;
            }
            this->resized();
        }
        if (key.getKeyCode() == 43 || key.getKeyCode() == 59) // "+ or ;"
        {
            auto fs = fragmentEditorComp.getFont();
            if (fs.getHeight() < 52)
            {
                fragmentEditorComp.setFont (Font (fs.getHeight() + 1.0f));
            }
        }
        if (key.getKeyCode() == 45) // "-"
        {
            auto fs = fragmentEditorComp.getFont();
            if (fs.getHeight() > 7)
            {
                fragmentEditorComp.setFont (Font (fs.getHeight() - 1.0f));
            }
        }
    }
    return true;
}

void GlslplugInAudioProcessorEditor::buttonClicked (Button* _button)
{
    if (_button->getName() == "SYNC")
    {
        setShaderSync();
    }
    if (_button->getName() == "AUTO_MODE")
    {
        isShaderSyncAuto = _button->getToggleState();
        setShaderSync();
    }
    if (_button->getName() == "PLAY_WND")
    {
        if (processor.existPlayerWindow())
            processor.deletePlayerWindow();
        else
            processor.createPlayerWindow();
    }
}
