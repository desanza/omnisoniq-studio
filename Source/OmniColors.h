#pragma once
#include <JuceHeader.h>

namespace OmniColors
{
    // Dark shell backgrounds
    inline const juce::Colour bgShell      { 0xff0b0e14 };
    inline const juce::Colour bgPanel      { 0xff0e1117 };
    inline const juce::Colour bgSurface    { 0xff12141b };
    inline const juce::Colour bgCard       { 0xff13161e };
    inline const juce::Colour bgTopBar     { 0xff0f1219 };

    // Borders
    inline const juce::Colour border       { 0xff1c202b };
    inline const juce::Colour borderLight  { 0xff1f2430 };
    inline const juce::Colour borderDim    { 0xff232936 };

    // Text
    inline const juce::Colour textPrimary  { 0xffe8ebf2 };
    inline const juce::Colour textSecond   { 0xff8b93a7 };
    inline const juce::Colour textTertiary { 0xff5a6175 };
    inline const juce::Colour textDim      { 0xff3c4354 };

    // Accent (purple)
    inline const juce::Colour accent       { 0xff7c6cff };
    inline juce::Colour accentAlpha (float a) { return accent.withAlpha(a); }

    // Stem colours
    inline const juce::Colour stemDrums    { 0xffff6b81 };
    inline const juce::Colour stemBass     { 0xffffa94d };
    inline const juce::Colour stemMelody   { 0xff51cf66 };
    inline const juce::Colour stemVocals   { 0xffe879f9 };
    inline const juce::Colour stemOther    { 0xff4dabf7 };

    // Status
    inline const juce::Colour success      { 0xff51cf66 };
    inline const juce::Colour warning      { 0xffffa94d };
    inline const juce::Colour error        { 0xffff6b81 };
}
