#include "ChoiceDialog.h"

// TODO: the OK button should be disabled when no option is selected
// TODO: add a static invocation, taking a QStringList

ChoiceDialog::ChoiceDialog (QWidget *parent, Qt::WindowFlags f):
	SkDialog<Ui::ChoiceDialogClass> (parent, f)
{
	ui.setupUi(this);

	ui.textLabel->setVisible (false);

	// It might be tempting to remove the dummy option radio button here, but
	// it is accessed by ui.retranslateUi. We therefore have to reuse it when
	// the first option is added.
}

ChoiceDialog::~ChoiceDialog()
{
}

/**
 * Shows a modal dialog with the given options
 *
 * The default index may be a valid index or -1 for no default.
 *
 * If the text is empty, the text won't be shown at all.
 *
 * The index of the chosen option is returned. If the dialog is canceled or no
 * option is selected, -1 is returned.
 */
int ChoiceDialog::choose (const QString &title, const QString &text,
	const QStringList &options, int defaultIndex,
	QWidget *parent, Qt::WindowFlags flags)
{
	// Create the dialog
	ChoiceDialog dialog (parent, flags);

	// Set the title and text
	dialog.setWindowTitle (title);
	if (!text.isEmpty ())
		dialog.setText (text);

	// Add the options
	foreach (const QString &text, options)
		dialog.addOption (text);

	// Select the default option, if in range
	if (defaultIndex>=0 && defaultIndex<options.size ())
		dialog.setSelectedOption (defaultIndex);

	// Show the dialog (modally)
	int result=dialog.exec ();

	// Return the result
	if (result==QDialog::Accepted)
		return dialog.getSelectedOption ();
	else
		return -1;
}

/**
 * Sets the text show above the options
 *
 * If this function is not called, the text label won't be shown at all.
 *
 * @param text the text to show; can be Qt rich text
 */
void ChoiceDialog::setText (const QString &text)
{
	ui.textLabel->setVisible (true);
	ui.textLabel->setText (text);
}

/**
 * Adds an option radio button
 *
 * @param text the text of the radio button; must be plain text
 */
void ChoiceDialog::addOption (const QString &text)
{
	QRadioButton *radioButton=addRadioButton ();
	radioButton->setText (text);
}

/**
 * Adds a radio button to the list
 *
 * If the first radio button in the GUI is not used yet, it is used. Otherwise,
 * a new radio button is created and added to the GUI and the layout.
 *
 * In any case, the radio button is added to the list of radio buttons and
 * returned.
 */
QRadioButton *ChoiceDialog::addRadioButton ()
{
	QRadioButton *radioButton;

	if (radioButtons.isEmpty ())
	{
		// No radio buttons have been added yet. For the first one, use the one
		// that's in the dialog by default.
		radioButton=ui.firstChoiceInput;
	}
	else
	{
		// There are already radio buttons, so the default button has already
		// been used. Add a new one to the parent widget and to the layout.
		radioButton=new QRadioButton (ui.optionsWidget);
		ui.optionsLayout->addWidget (radioButton);
	}

	// Add the radio button to the list of radio buttons
	radioButtons.append (radioButton);

	return radioButton;
}

/**
 * Sets the index of the selected option
 *
 * The value must be greater or equal to 0 and less than the number of options
 * added so far. Otherwise, the result is undefined.
 */
void ChoiceDialog::setSelectedOption (int option)
{
	// Invalid value - return
	if (option<0 || option>=radioButtons.size ())
		return;

	// Check the radio button
	radioButtons[option]->setChecked (true);

	// Set the input focus
	radioButtons[option]->setFocus ();
}

/**
 * Returns the index of the currently selected option, or -1 if none
 */
int ChoiceDialog::getSelectedOption ()
{
	for (int i=0, n=radioButtons.size (); i<n; ++i)
		if (radioButtons[i]->isChecked())
			return i;

	return -1;
}
