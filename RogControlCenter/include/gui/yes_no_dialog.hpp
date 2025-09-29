#pragma once

#include <qwidget.h>

#include <string>

class YesNoDialog {
  public:
	/**
	 * @brief Displays a modal dialog with a Yes/No question.
	 *
	 * Presents a dialog window with the specified title and body text, allowing the user to choose between "Yes" and "No".
	 *
	 * @param title The title of the dialog window.
	 * @param body The message or question to display in the dialog.
	 * @param parent The parent widget of the dialog. Defaults to nullptr if not specified.
	 * @return true if the user selects "Yes", false if "No" is selected or the dialog is closed.
	 */
	static bool showDialog(std::string title, std::string body, QWidget* parent = nullptr);
};
