#pragma once
#include "JuceHeader_Wrapper.h"
#include <functional>

#define CHANNELFORMAT_ALLCHANNELS_ID 0
#define PACKFORMAT_UNSET_ID 0

class AdmStemPluginAudioProcessor;

class AudioParameterIntWithInternalSet : public AudioParameterInt
{
public:
    AudioParameterIntWithInternalSet(const String& parameterID, const String& name,
                                     int minValue, int maxValue, int defaultValue,
                                     const String& label = String(""),
                                     std::function<String(int, int)> stringFromInt = nullptr,
                                     std::function<int(const String&)> intFromString = nullptr)
        : AudioParameterInt(parameterID, name, minValue, maxValue, defaultValue, label, stringFromInt, intFromString) {};
    bool wasChangeInvokedInternally() { return latestChangeInvokedInternally; }
    void internalSetIntAndNotifyHost(int newValue) {
        setViaInternalSet = true;
        setValueNotifyingHost(convertTo0to1(newValue));
    }

private:
    bool setViaInternalSet{ false };
    bool latestChangeInvokedInternally{ false };
    void valueChanged(int newValue) override {
        // Called before listeners
        latestChangeInvokedInternally = setViaInternalSet;
        setViaInternalSet = false; //reset
    };

};

class NonAutoAudioParameterInt : public AudioParameterIntWithInternalSet
{
public:
    NonAutoAudioParameterInt(const String& parameterID, const String& name,
        int minValue, int maxValue,
        int defaultValue,
        const String& label = String(),
        std::function<String(int value, int maximumStringLength)> stringFromInt = nullptr,
        std::function<int(const String& text)> intFromString = nullptr)
        : AudioParameterIntWithInternalSet(parameterID, name, minValue, maxValue, defaultValue, label, stringFromInt, intFromString) {}

    bool isAutomatable() const override {
        return false;
    };
};

class NonAutoAudioParameterBool : public AudioParameterBool
{
public:
    NonAutoAudioParameterBool(const String& parameterID, const String& name, bool defaultValue,
        const String& label = String(),
        std::function<String(bool value, int maximumStringLength)> stringFromBool = nullptr,
        std::function<bool(const String& text)> boolFromString = nullptr)
        : AudioParameterBool(parameterID, name, defaultValue, label, stringFromBool, boolFromString) {}

    bool isAutomatable() const override {
        return false;
    };
};

class ReadOnlyAudioParameterInt : public AudioParameterInt
{
public:
    ReadOnlyAudioParameterInt(const String& parameterID, const String& name,
        int minValue, int maxValue,
        int defaultValue,
        const String& label = String(),
        std::function<String(int value, int maximumStringLength)> stringFromInt = nullptr,
        std::function<int(const String& text)> intFromString = nullptr)
        : AudioParameterInt(parameterID, name, minValue, maxValue, defaultValue, label, stringFromInt, intFromString) {
        roResetting = false;
        roValue = convertTo0to1((float)defaultValue);
        roValueInt = defaultValue;
    }

    bool isAutomatable() const override {
        return false;
    };

    void internalSetIntAndNotifyHost(int newValue) {
        roValue = convertTo0to1((float)newValue);
        roValueInt = newValue;
        setValueNotifyingHost(roValue);
    }

private:
    // Can not simply override the set method with blank as it is also used internally. Can not use custom set code as the underlying 'value' var is inaccessible (private in base class).
    // Therefore, best we can do is to just reset it if it is changed.
    void valueChanged(int newValue) {
        if (!roResetting && newValue != roValueInt) {
            roResetting = true;
            setValueNotifyingHost(roValue);
            roResetting = false;
        }
    }

    float roValue;
    int roValueInt;
    bool roResetting;

};

class ParamChangeCallback : public AudioProcessorParameter::Listener
{
public:
    ParamChangeCallback(std::function<void()> func) : callback{ func } {}
    ~ParamChangeCallback() {}
    void parameterGestureChanged(int parameterIndex, bool gestureIsStarting) override {}
    void parameterValueChanged(int parameterIndex, float newValue) override { callback(); }
private:
    std::function<void()> callback;
};