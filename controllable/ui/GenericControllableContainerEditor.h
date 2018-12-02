/*
  ==============================================================================

    GenericControllableContainerEditor.h
    Created: 9 May 2016 6:41:59pm
    Author:  bkupe

  ==============================================================================
*/

#pragma once


class GenericControllableContainerEditor :
	public InspectableEditor,
	public ContainerAsyncListener,
	public Button::Listener,
	public ChangeListener,
	public Label::Listener
{
public:
	GenericControllableContainerEditor(WeakReference<Inspectable> _inspectable, bool isRoot, bool buildAtCreation = true);
	virtual ~GenericControllableContainerEditor();

	
	int headerHeight;
	const int headerGap = 4;

	bool isRebuilding; //to avoid constant resizing when rebuilding/adding items
	
	bool prepareToAnimate;
	ComponentAnimator collapseAnimator;

	Colour contourColor;
	Label containerLabel;

	WeakReference<ControllableContainer> container;
	OwnedArray<InspectableEditor> childEditors;

	ScopedPointer<ImageButton> expandBT;
	ScopedPointer<ImageButton> collapseBT;
	ScopedPointer<ImageButton> addBT;
	Component headerSpacer;

	virtual void setCollapsed(bool value, bool force = false, bool animate = true, bool doNotRebuild = false);
	virtual void resetAndBuild();

	void paint(Graphics &g) override;
	void resized() override;
	virtual void resizedInternal(juce::Rectangle<int> &r);
	virtual void resizedInternalHeader(juce::Rectangle<int> &r);
	virtual void resizedInternalContent(juce::Rectangle<int> &r);
	void clear();

	void mouseDown(const MouseEvent &e) override;

	juce::Rectangle<int> getHeaderBounds(); 
	juce::Rectangle<int> getContentBounds();
	
	bool canBeCollapsed();

	void addControllableUI(Controllable * c, bool resize = false);
	void removeControllableUI(Controllable *c, bool resize = false);

	virtual InspectableEditor * addEditorUI(ControllableContainer * cc, bool resize = false);
	virtual void removeEditorUI(ControllableContainer * cc, bool resize = false);
	
	virtual void showMenuAndAddControllable();

	InspectableEditor * getEditorForInspectable(Inspectable * i);
	
	virtual void buttonClicked(Button * b) override;
	virtual void labelTextChanged(Label * l) override;

	void newMessage(const ContainerAsyncEvent & p) override;
	virtual void controllableFeedbackUpdate(Controllable *) {};
	void childBoundsChanged(Component *) override;



	// Inherited via ChangeListener
	virtual void changeListenerCallback(ChangeBroadcaster * source) override;

	class  ContainerEditorListener
	{
	public:
		/** Destructor. */
		virtual ~ContainerEditorListener() {}
		virtual void containerRebuilt(GenericControllableContainerEditor *) {}
	};

	ListenerList<ContainerEditorListener> containerEditorListeners;
	void addContainerEditorListener(ContainerEditorListener* newListener) { containerEditorListeners.add(newListener); }
	void removeContainerEditorListener(ContainerEditorListener* listener) { containerEditorListeners.remove(listener); }
};


class EnablingControllableContainerEditor :
	public GenericControllableContainerEditor
{
public:
	EnablingControllableContainerEditor(EnablingControllableContainer * cc, bool isRoot, bool buildAtCreation = true);
	~EnablingControllableContainerEditor() {}

	EnablingControllableContainer * ioContainer;
	ScopedPointer<BoolImageToggleUI> enabledUI;

	virtual void resizedInternalHeader(juce::Rectangle<int> &r) override;
	virtual void controllableFeedbackUpdate(Controllable *) override;
};