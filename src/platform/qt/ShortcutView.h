/* Copyright (c) 2013-2015 Jeffrey Pfau
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#pragma once

#include "GamepadAxisEvent.h"
#include "InputIndex.h"
#include "InputModel.h"

#include <QWidget>

#include "ui_ShortcutView.h"

namespace QGBA {

class InputController;

class ShortcutView : public QWidget {
Q_OBJECT

public:
	ShortcutView(QWidget* parent = nullptr);
	~ShortcutView();

	void setModel(InputIndex* model);
	void setInputController(InputController* input);

	const InputIndex* root() { return m_model.inputIndex(); }

protected:
	virtual bool event(QEvent*) override;
	virtual void closeEvent(QCloseEvent*) override;

private slots:
	void load(const QModelIndex&);
	void clear();
	void updateButton(int button);
	void updateAxis(int axis, int direction);
	void updateGamepads();

private:
	Ui::ShortcutView m_ui;

	InputModel m_model;
	InputController* m_input = nullptr;
};

}
