#include "StemLane.h"

StemLane::StemLane (OmnisoniqProcessor& proc, int stemIndex)
    : mProc (proc), mIdx (stemIndex)
{
    addAndMakeVisible (mSoloBtn);
    addAndMakeVisible (mMuteBtn);
    addAndMakeVisible (mVolSlider);
    addAndMakeVisible (mWaveform);

    mSoloBtn.setClickingTogglesState (true);
    mMuteBtn.setClickingTogglesState (true);

    mSoloBtn.onClick = [this] { mProc.setStemSolo (mIdx, mSoloBtn.getToggleState()); };
    mMuteBtn.onClick = [this] { mProc.setStemMute (mIdx, mMuteBtn.getToggleState()); };

    mVolSlider.setSliderStyle (juce::Slider::LinearHorizontal);
    mVolSlider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    mVolSlider.setRange (0.0, 1.0, 0.01);
    mVolSlider.setValue (0.8);
    mVolSlider.onValueChange = [this] {
        mProc.setStemVolume (mIdx, (float)mVolSlider.getValue());
    };

    mWaveform.onSeek = [this] (float norm) { mProc.seekTo (norm); };
    mWaveform.setShowGrid (true);

    startTimerHz (30);
}

StemLane::~StemLane() { stopTimer(); }

void StemLane::resized()
{
    auto b = getLocalBounds();

    // Left header: 160px
    auto header = b.removeFromLeft (160);
    b.removeFromLeft (8); // gap

    // Within header:
    auto topRow = header.removeFromTop (24);
    mSoloBtn.setBounds (topRow.removeFromRight (20).reduced (1));
    mMuteBtn.setBounds (topRow.removeFromRight (20).reduced (1));
    // label space left over (painted in paint())

    header.removeFromTop (4);
    mVolSlider.setBounds (header);

    mWaveform.setBounds (b);
}

void StemLane::paint (juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat();

    // Left card background
    auto card = b.removeFromLeft (160);
    b.removeFromLeft (8);

    StemState* s = nullptr;
    if (juce::isPositiveAndBelow (mIdx, mProc.state.stems.size()))
        s = mProc.state.stems[mIdx];

    juce::Colour colour = s ? s->colour : OmniColors::accent;

    // Card
    g.setColour (OmniColors::bgCard);
    g.fillRoundedRectangle (card, 9.0f);
    g.setColour (OmniColors::borderLight);
    g.drawRoundedRectangle (card, 9.0f, 1.0f);

    // Accent left border
    auto leftBorder = card.withWidth (3.0f);
    g.setColour (colour);
    g.fillRoundedRectangle (leftBorder, 3.0f);

    // Stem name
    g.setColour (OmniColors::textPrimary);
    g.setFont (juce::Font (12.0f, juce::Font::bold));
    auto nameArea = card.removeFromTop (24.0f).withLeft (card.getX() + 8.0f)
                        .withRight (card.getRight() - 44.0f);
    g.drawText (s ? s->name : "Stem", nameArea.toNearestInt(), juce::Justification::centredLeft);

    // Waveform background
    g.setColour (OmniColors::bgSurface);
    g.fillRoundedRectangle (b, 9.0f);
    g.setColour (OmniColors::borderLight);
    g.drawRoundedRectangle (b, 9.0f, 1.0f);
}

void StemLane::update()
{
    if (!juce::isPositiveAndBelow (mIdx, mProc.state.stems.size())) return;
    auto* s = mProc.state.stems[mIdx];

    mSoloBtn.setToggleState (s->soloed, juce::dontSendNotification);
    mMuteBtn.setToggleState (s->muted,  juce::dontSendNotification);

    // Update waveform colour
    mWaveform.setColour (s->colour);

    bool anySolo = false;
    for (auto* st : mProc.state.stems)
        if (st->soloed) { anySolo = true; break; }
    bool audible = anySolo ? s->soloed : !s->muted;
    mWaveform.setDim (!audible);

    if (s->loaded && mWaveform.onSeek) // Check if audio loaded — rebuild peaks once
    {
        // Peaks may already be set via update() calls; only rebuild when needed
        // (We set them once when stem becomes loaded)
        mWaveform.setAudio (s->audio, mProc.state.masterSampleRate);
    }
}

void StemLane::timerCallback()
{
    if (!juce::isPositiveAndBelow (mIdx, mProc.state.stems.size())) return;
    mWaveform.setPlayPos (mProc.state.playPos.load());
}
