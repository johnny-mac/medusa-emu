/* Copyright (c) 2013-2017 Jeffrey Pfau
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#include "InputIndex.h"

#include "ConfigController.h"

using namespace QGBA;

void InputIndex::setConfigController(ConfigController* controller) {
	m_config = controller;
	for (auto& item : m_items) {
		loadShortcuts(item);
	}
}

void InputIndex::clone(InputIndex* root, bool actions) {
	if (!actions) {
		clone(const_cast<const InputIndex*>(root));
	} else {
		qDeleteAll(m_items);
		m_items.clear();
		for (auto& item : root->m_items) {
			InputItem* newItem = new InputItem(*item);
			m_items.append(newItem);
			itemAdded(newItem);
		}
	}
	rebuild();
}

void InputIndex::clone(const InputIndex* root) {
	qDeleteAll(m_items);
	m_items.clear();
	for (auto& item : root->m_items) {
		InputItem* newItem = new InputItem(*item);
		m_items.append(newItem);
		itemAdded(newItem);
	}
	rebuild();
}

void InputIndex::rebuild(const InputIndex* root) {
	if (!root) {
		root = this;
	}

	m_names.clear();
	m_menus.clear();
	m_shortcuts.clear();
	m_buttons.clear();
	m_axes.clear();

	for (auto& item : root->m_items) {
		InputItem* newItem = nullptr;
		for (auto &iter : m_items) {
			if (*iter == *item) {
				newItem = iter;
				break;
			}
		}
		if (!newItem) {
			continue;
		}
		if (item->hasShortcut()) {
			newItem->setShortcut(item->shortcut());
		}
		if (item->hasButton()) {
			newItem->setButton(item->button());
		}
		if (item->hasAxis()) {
			newItem->setAxis(item->axis(), item->direction());
		}

		itemAdded(newItem);
	}
}

InputItem* InputIndex::itemAt(const QString& name) {
	return m_names[name];
}

const InputItem* InputIndex::itemAt(const QString& name) const {
	return m_names[name];
}

InputItem* InputIndex::itemForMenu(const QMenu* menu) {
	InputItem* item = m_menus[menu];
	return item;
}

const InputItem* InputIndex::itemForMenu(const QMenu* menu) const {
	const InputItem* item = m_menus[menu];
	return item;
}

InputItem* InputIndex::itemForShortcut(int shortcut) {
	return m_shortcuts[shortcut];
}

InputItem* InputIndex::itemForButton(int button) {
	return m_buttons[button];
}

InputItem* InputIndex::itemForAxis(int axis, GamepadAxisEvent::Direction direction) {
	return m_axes[qMakePair(axis, direction)];
}

bool InputIndex::loadShortcuts(InputItem* item) {
	if (item->name().isNull()) {
		return false;
	}
	loadGamepadShortcuts(item);
	QVariant shortcut = m_config->getQtOption(item->name(), KEY_SECTION);
	if (!shortcut.isNull()) {
		if (shortcut.toString().endsWith("+")) {
			item->setShortcut(toModifierShortcut(shortcut.toString()));
		} else {
			item->setShortcut(QKeySequence(shortcut.toString())[0]);
		}
		return true;
	}
	return false;
}

void InputIndex::loadGamepadShortcuts(InputItem* item) {
	if (item->name().isNull()) {
		return;
	}
	QVariant button = m_config->getQtOption(item->name(), BUTTON_SECTION);
	if (!button.isNull()) {
		item->setButton(button.toInt());
	}

	QVariant axis = m_config->getQtOption(item->name(), AXIS_SECTION);
	int oldAxis = item->axis();
	if (oldAxis >= 0) {
		item->setAxis(-1, GamepadAxisEvent::NEUTRAL);
	}
	if (!axis.isNull()) {
		QString axisDesc = axis.toString();
		if (axisDesc.size() >= 2) {
			GamepadAxisEvent::Direction direction = GamepadAxisEvent::NEUTRAL;
			if (axisDesc[0] == '-') {
				direction = GamepadAxisEvent::NEGATIVE;
			}
			if (axisDesc[0] == '+') {
				direction = GamepadAxisEvent::POSITIVE;
			}
			bool ok;
			int axis = axisDesc.mid(1).toInt(&ok);
			if (ok) {
				item->setAxis(axis, direction);
			}
		}
	}
}

int InputIndex::toModifierShortcut(const QString& shortcut) {
	// Qt doesn't seem to work with raw modifier shortcuts!
	QStringList modifiers = shortcut.split('+');
	int value = 0;
	for (const auto& mod : modifiers) {
		if (mod == QLatin1String("Shift")) {
			value |= Qt::ShiftModifier;
			continue;
		}
		if (mod == QLatin1String("Ctrl")) {
			value |= Qt::ControlModifier;
			continue;
		}
		if (mod == QLatin1String("Alt")) {
			value |= Qt::AltModifier;
			continue;
		}
		if (mod == QLatin1String("Meta")) {
			value |= Qt::MetaModifier;
			continue;
		}
	}
	return value;
}

void InputIndex::itemAdded(InputItem* child) {
	const QMenu* menu = child->menu();
	if (menu) {
		m_menus[menu] = child;
	}
	m_names[child->name()] = child;

	if (child->shortcut() > 0) {
		m_shortcuts[child->shortcut()] = child;
	}
	if (child->button() >= 0) {
		m_buttons[child->button()] = child;
	}
	if (child->direction() != GamepadAxisEvent::NEUTRAL) {
		m_axes[qMakePair(child->axis(), child->direction())] = child;
	}
}

bool InputIndex::isModifierKey(int key) {
	switch (key) {
	case Qt::Key_Shift:
	case Qt::Key_Control:
	case Qt::Key_Alt:
	case Qt::Key_Meta:
		return true;
	default:
		return false;
	}
}

int InputIndex::toModifierKey(int key) {
	int modifiers = key & (Qt::ShiftModifier | Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier);
	key ^= modifiers;
	switch (key) {
	case Qt::Key_Shift:
		modifiers |= Qt::ShiftModifier;
		break;
	case Qt::Key_Control:
		modifiers |= Qt::ControlModifier;
		break;
	case Qt::Key_Alt:
		modifiers |= Qt::AltModifier;
		break;
	case Qt::Key_Meta:
		modifiers |= Qt::MetaModifier;
		break;
	default:
		break;
	}
	return modifiers;
}

void InputIndex::saveConfig() {
	for (auto& item : m_items) {
		if (item->hasShortcut()) {
			m_config->setQtOption(item->name(), QKeySequence(item->shortcut()).toString(), KEY_SECTION);
		}
		if (item->hasButton()) {
			m_config->setQtOption(item->name(), item->button(), BUTTON_SECTION);
		}
		if (item->hasAxis()) {
			m_config->setQtOption(item->name(), QString("%1%2").arg(GamepadAxisEvent::POSITIVE ? '+' : '-').arg(item->axis()), AXIS_SECTION);
		}
	}
}
