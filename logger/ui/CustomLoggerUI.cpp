
static String EmptyString;
void CustomLoggerUI::newMessage(const String& s)
{
	LogElement* el = new LogElement(s);



	logElements.add(el);
	totalLogRow += el->getNumLines();


	//bool overFlow = false;

	//coalesce messages
	if (!Timer::isTimerRunning()) {
		startTimer(100);
	}

};
void CustomLoggerUI::timerCallback()
{
	stopTimer();
	if (totalLogRow.get() > maxNumElement)
	{
		int curCount = 0;
		int idxToRemove = -1;

		for (int i = logElements.size() - 1; i >= 0; i--)
		{
			curCount += logElements[i]->getNumLines();

			if (curCount >= maxNumElement)
			{
				if (curCount != maxNumElement)
				{
					logElements[i]->trimToFit(logElements[i]->getNumLines() - (curCount - maxNumElement));
				}

				idxToRemove = i - 1;
				break;
			}

		}

		if (idxToRemove >= 0)logElements.removeRange(0, idxToRemove + 1);

		totalLogRow = maxNumElement;


	}
	//DBG("Handle Async Update");
//    auto cTime = Time::getMillisecondCounter();
//    if(cTime - lastUpdateTime < 500 ){
//        triggerAsyncUpdate();
//    }
//    else{
//        lastUpdateTime = cTime;

	logListComponent->updateContent();
	logListComponent->scrollToEnsureRowIsOnscreen(totalLogRow.get() - 1);
#if USE_CACHED_GLYPH
	logList.cleanUnusedGlyphs();
#endif
	repaint();


	//    }
}

CustomLoggerUI::CustomLoggerUI(const String& contentName, CustomLogger * l) :
	ShapeShifterContentComponent(contentName),
	logger(l),
	logList(this),
	maxNumElement(100),
	totalLogRow(0),
	lastUpdateTime(0)
{

	logger->addLogListener(this);
	TableHeaderComponent* thc = new TableHeaderComponent();
	thc->addColumn(juce::translate("Time"), 1, 60);
	thc->addColumn(juce::translate("Source"), 2, 80);
	thc->addColumn(juce::translate("Content"), 3, 400);


	logListComponent = new TableListBox("CustomLogger", &logList);
	logListComponent->setOpaque(true);
	logListComponent->setRowHeight(13);
	logListComponent->setHeaderHeight(20);
	logListComponent->getViewport()->setScrollBarThickness(10);

	logListComponent->setColour(TableListBox::backgroundColourId, findColour(ResizableWindow::backgroundColourId));
	logListComponent->setHeader(thc);
	addAndMakeVisible(logListComponent);

	LOG(l->getWelcomeMessage());
#if USE_FILE_LOGGER
	LOG(juce::translate("please provide logFile for any bug report :\nlogFile in 123").replace("123", l->fileWriter.getFilePath()));
#endif
	clearB.setButtonText(juce::translate("Clear"));
	clearB.addListener(this);
	addAndMakeVisible(clearB);

	copyB.setButtonText(juce::translate("Copy All to Clipboard"));
	copyB.addListener(this);
	addAndMakeVisible(copyB);
	logListComponent->setMouseCursor(MouseCursor::IBeamCursor);

	logListComponent->setMultipleSelectionEnabled(true);
	setInterceptsMouseClicks(true, false);
	addMouseListener(this, true);

}


CustomLoggerUI::~CustomLoggerUI()
{
	stopTimer();
	//        logListComponent.setModel(nullptr);
	logger->removeLogListener(this);
}

void CustomLoggerUI::resized()
{

	ShapeShifterContentComponent::resized();
	juce::Rectangle<int> area = getLocalBounds().withTop(5);
	auto footer = area.removeFromBottom(30).reduced(5);
	clearB.setBounds(footer.removeFromLeft(footer.getWidth() / 2).reduced(2));
	copyB.setBounds(footer.reduced(2));
	logListComponent->setBounds(area);
	bool firstVisible = area.getWidth() > 400;
	logListComponent->getHeader().setColumnVisible(1, firstVisible);
	bool secondVisible = area.getWidth() > 300;
	logListComponent->getHeader().setColumnVisible(2, secondVisible);

	int tw = getWidth();

	if (firstVisible)tw -= logListComponent->getHeader().getColumnWidth(1);

	if (secondVisible)tw -= logListComponent->getHeader().getColumnWidth(2);

	tw -= logListComponent->getViewport()->getScrollBarThickness();
	tw = jmax(tw, 100);
	logListComponent->getHeader().setColumnWidth(3, tw);
}



void CustomLoggerUI::updateTotalLogRow()
{
	totalLogRow = 0;

	for (auto& l : logElements)
	{
		totalLogRow += l->getNumLines();
	}

}
const String& CustomLoggerUI::getSourceForRow(const int r) const
{
	if (auto el = getElementForRow(r)) {
		return el->source;
	}
	return EmptyString;
}
const bool CustomLoggerUI::isPrimaryRow(const int r) const
{
	int count = 0;
	int idx = 0;

	while (count <= r && idx < logElements.size())
	{
		if (count == r)
		{
			return true;
		}
		count += logElements.getUnchecked(idx)->getNumLines();
		idx++;

	}

	return false;
}

const String&   CustomLoggerUI::getContentForRow(const int r) const
{
	int count = 0;
	int idx = 0;

	while (idx < logElements.size())
	{

		int nl = logElements.getUnchecked(idx)->getNumLines();

		if (count + nl > r)
		{
			return logElements.getUnchecked(idx)->getLine(r - count);
		}

		count += nl;
		idx++;
	}

	return EmptyString;
};

const LogElement* CustomLoggerUI::getElementForRow(const int r) const {
	int count = 0;
	int idx = 0;

	while (idx < logElements.size())
	{
		auto el = logElements.getUnchecked(idx);

		int nl = el->getNumLines();

		if (count + nl > r)
		{
			return el;
		}

		count += nl;
		idx++;
	}

	return nullptr;

}

const String  CustomLoggerUI::getTimeStringForRow(const int r) const
{
	if (auto el = getElementForRow(r)) {
		return String(el->time.toString(false, true, true, true));
	}

	return "";
};

const Colour& CustomLoggerUI::getSeverityColourForRow(const int r) const
{

	if (auto el = getElementForRow(r))
	{
		LogElement::Severity s = el->severity;

		switch (s)
		{
		case LogElement::LOG_NONE:
			return logNoneColor;

		case LogElement::LOG_DBG:
			return logDbgColor;

		case LogElement::LOG_WARN:
			return Colours::orange;

		case LogElement::LOG_ERR:
			return Colours::red;

		default:
			return Colours::pink;

		}

	}

	return Colours::pink;
};


bool CustomLoggerUI::keyPressed(const KeyPress& k) {

	if (k == KeyPress('c', ModifierKeys::commandModifier, 0)) {
		String textToCopy;
		Array<Range<int>> ranges = logListComponent->getSelectedRows().getRanges();
		for (auto &r : ranges) {
			for (int i = r.getStart(); i < r.getEnd(); i++) {
				StringArray arr;
				for (int c = 1; c <= 3; c++) {
					if (logListComponent->getHeader().isColumnVisible(c)) {
						arr.add(logList.getTextAt(i, c));
					}
				}
				textToCopy += arr.joinIntoString("\t") + "\n";
			}
		}
		if (textToCopy.isNotEmpty()) {
			SystemClipboard::copyTextToClipboard(textToCopy);
			return true;
		}
	}
	return false;
}

void CustomLoggerUI::mouseDown(const MouseEvent& me) {
	auto pos = me.getEventRelativeTo(logListComponent);
	auto rowUnderMouse = logListComponent->getRowContainingPosition(pos.x, pos.y);
	logListComponent->selectRow(rowUnderMouse);
	grabKeyboardFocus();

};

void CustomLoggerUI::mouseDrag(const MouseEvent& me) {
	auto pos = me.getEventRelativeTo(logListComponent);
	auto rowUnderMouse = logListComponent->getRowContainingPosition(pos.x, pos.y);
	auto rowStart = logListComponent->getRowContainingPosition(pos.getMouseDownX(), pos.getMouseDownY());
	logListComponent->selectRangeOfRows(rowStart, rowUnderMouse);

};

MouseCursor  CustomLoggerUI::getMouseCursor() {
	return MouseCursor::IBeamCursor;
}


//////////////
// logList

CustomLoggerUI::LogList::LogList(CustomLoggerUI* o) : owner(o), minRow(0), maxRow(0)
{
}

int CustomLoggerUI::LogList::getNumRows()
{

	return owner->totalLogRow.get();
};

void CustomLoggerUI::LogList::paintRowBackground(Graphics& g,
	int rowNumber,
	int width, int height,
	bool isSelected)
{
	Colour c = owner->getSeverityColourForRow(rowNumber).darker(2);// BG_COLOR.brighter(.1f);// (rowNumber);
	if (rowNumber % 2 == 0) c = c.brighter(.05f);

	g.setColour(c);
	g.fillRect(0, 0, width, height);
};


// use as function to prevent juce leak detection
const Font  getLogFont() {
	static Font  f(12);
	return f;
}

String CustomLoggerUI::LogList::getTextAt(int rowNumber, int columnId) {
	String text;


	switch (columnId)
	{
	case 1:
		text = owner->isPrimaryRow(rowNumber) ? owner->getTimeStringForRow(rowNumber) : "";
		break;

	case 2:
		text = owner->isPrimaryRow(rowNumber) ? owner->getSourceForRow(rowNumber) : "";
		break;

	case 3:
		text = owner->getContentForRow(rowNumber);
		break;
	}
	return text;

}
#if LOGGER_USE_LABEL
Component * CustomLoggerUI::LogList::refreshComponentForCell(int rowNumber, int columnId, bool isRowSelected,
	Component* existingComponentToUpdate) {
	Colour color = owner->findColour(Label::textColourId);
	String text = getTextAt(rowNumber, columnId);
	Label * lp = nullptr;

	if (existingComponentToUpdate) {
		lp = dynamic_cast<Label*>(existingComponentToUpdate);

	}
	else {
		lp = new Label();
		lp->setFont(getLogFont());
		lp->setEditable(true);
		//        lp->showEditor();

	}
	jassert(lp);
	if (lp) {
		lp->setText(text, dontSendNotification);
	}
	return lp;
}
#endif
void CustomLoggerUI::LogList::paintCell(Graphics& g,
	int rowNumber,
	int columnId,
	int width, int height,
	bool)
{

	g.setColour(owner->getSeverityColourForRow(rowNumber));


#if !LOGGER_USE_LABEL
	String text = getTextAt(rowNumber, columnId);

#if USE_CACHED_GLYPH
	if (cachedG.contains(text)) {
		auto & cg = cachedG.getReference(text);
		cg.paint(g);
		return;
	}


	auto & cg = cachedG.getReference(text);
	cg.setFont(getLogFont());
	cg.setText(text);
	cg.setSize(width, height);
	cg.paint(g);
#else
	g.setFont(getLogFont());
	g.drawFittedText(text, 0, 0, width, height, Justification::left, 1);
#endif

#endif
};

String CustomLoggerUI::LogList::getCellTooltip(int rowNumber, int /*columnId*/)
{
	auto el = owner->getElementForRow(rowNumber);

	String sR = el->source;
	return
		(sR.isNotEmpty() ?
			sR + " (" + el->time.toString(false, true, true, true) + ")" + "\n" : "")
		+ (el->getNumLines() < 10 ? el->content : owner->getSourceForRow(rowNumber));


};

#if USE_CACHED_GLYPH
void CustomLoggerUI::LogList::cleanUnusedGlyphs() {
	int nminRow = owner->logListComponent->getRowContainingPosition(1, 1);
	int nmaxRow = owner->logListComponent->getRowContainingPosition(1, owner->logListComponent->getHeight());
	if (nminRow == -1)return;
	if (nmaxRow == -1)nmaxRow = owner->totalLogRow.get();

	int min = 0, max = 0;
	if (nminRow > minRow) {
		min = minRow; max = nminRow;
	}
	if (nmaxRow < maxRow) {
		min = nmaxRow; max = maxRow;
	}

	for (int i = min; i < max; i++) {
		cachedG.remove(owner->getContentForRow(i));
		if (owner->isPrimaryRow(i)) {
			cachedG.remove(owner->getSourceForRow(i));
			cachedG.remove(owner->getTimeStringForRow(i));
		}
	}

	minRow = nminRow;
	maxRow = nmaxRow;


}
#endif

void CustomLoggerUI::buttonClicked(Button* b)
{

	if (b == &clearB)
	{
		logElements.clear();
		totalLogRow = 0;
		logListComponent->updateContent();
		LOG(juce::translate("Cleared."));
	}

	else if (b == &copyB) {
		String s;
		for (auto & el : logElements) {
			int leftS = el->source.length() + 3;
			s += el->source + " : ";
			for (int i = 0; i < el->getNumLines(); i++) {
				if (i != 0)for (int j = 0; j < leftS; j++) s += " ";
				s += el->getLine(i) + "\n";
			}
		}
		SystemClipboard::copyTextToClipboard(s);
	}
}