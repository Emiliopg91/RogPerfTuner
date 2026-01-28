#include "gui/changelog_view.hpp"

#include <qnamespace.h>

ChangelogView::ChangelogView(QWidget* parent) : QDialog(parent) {
	setWindowTitle(translator.translate("changelog").c_str());
	setFixedSize(600, 600);

	auto* layout = new QVBoxLayout(this);

	changelogViewer = new QTextBrowser(this);
	changelogViewer->setOpenExternalLinks(false);

	updateButton = new QPushButton(translator.translate("update.now").c_str(), this);
	connect(updateButton, &QPushButton::clicked, this, &ChangelogView::onUpdateClicked);

	layout->addWidget(changelogViewer);
	layout->addWidget(updateButton, 0, Qt::AlignCenter);

	changelogViewer->setHtml(applicationService.getChangeLog().value_or(translator.translate("no.changelog.available")).c_str());
}

void ChangelogView::onUpdateClicked() {
	std::thread([this]() {
		applicationService.applyUpdate();
	}).detach();
	this->close();
}