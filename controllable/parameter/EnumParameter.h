/*
  ==============================================================================

    EnumParameter.h
    Created: 29 Sep 2016 5:34:59pm
    Author:  bkupe

  ==============================================================================
*/

#ifndef ENUMPARAMETER_H_INCLUDED
#define ENUMPARAMETER_H_INCLUDED

class EnumParameterUI;


class EnumParameter : public Parameter
{
public:

	EnumParameter(const String &niceName, const String &description, bool enabled = true);
	~EnumParameter() {}

	EnumParameter * addOption(String key, var data, bool selectIfFirstOption = true); //daisy chain
	void removeOption(String key);
	void clearOptions();

	void updateArgDescription();

	struct EnumValue
	{
		EnumValue(String key, var value) : key(key), value(value) {}
		String key;
		var value;
	};

	OwnedArray<EnumValue> enumValues;

	var getValueData();

	template<class T>
	T getValueDataAsEnum() { return (T)(int)enumValues[value]->value; }
	String getValueKey() { return stringValue(); }

	int getIndexForKey(StringRef key);
	EnumValue * getEntryForKey(StringRef key);

	StringArray getAllKeys();

	void setValueWithData(var data);
	void setValueWithKey(String data);
	void setNext(bool loop = true, bool addToUndo = false);

	bool checkValueIsTheSame(var oldValue, var newValue) override;
	
	static var getValueDataFromScript(const juce::var::NativeFunctionArgs &a);


	EnumParameterUI * createUI(EnumParameter * target = nullptr);
	ControllableUI * createDefaultUI(Controllable * targetControllable = nullptr) override;


	//Listener
	class  Listener
	{
	public:
		/** Destructor. */
		virtual ~Listener() {}
		virtual void enumOptionAdded(EnumParameter *, const String &) = 0;
		virtual void enumOptionRemoved(EnumParameter *, const String &) = 0;
	};

	ListenerList<Listener> enumListeners;
	void addEnumParameterListener(Listener* newListener) { enumListeners.add(newListener); }
	void removeEnumParameterListener(Listener* listener) { enumListeners.remove(listener); }


	static EnumParameter * create() { return new EnumParameter("new Enum Parameter",""); }
	virtual String getTypeString() const override { return getTypeStringStatic(); }
	static String getTypeStringStatic() { return "Enum"; }

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EnumParameter)
};


#endif  // ENUMPARAMETER_H_INCLUDED
