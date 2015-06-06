// samplv1widget_control.cpp
//
/****************************************************************************
   Copyright (C) 2012-2015, rncbc aka Rui Nuno Capela. All rights reserved.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along
   with this program; if not, write to the Free Software Foundation, Inc.,
   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

*****************************************************************************/

#include "samplv1widget_control.h"
#include "samplv1widget_controls.h"

#include <QMessageBox>
#include <QPushButton>
#include <QLineEdit>


//----------------------------------------------------------------------------
// samplv1widget_control -- UI wrapper form.

// Kind of singleton reference.
samplv1widget_control *samplv1widget_control::g_pInstance = NULL;


// Constructor.
samplv1widget_control::samplv1widget_control (
	QWidget *pParent, Qt::WindowFlags wflags )
	: QDialog(pParent, wflags)
{
	// Setup UI struct...
	m_ui.setupUi(this);

	// Make it auto-modeless dialog...
	QDialog::setAttribute(Qt::WA_DeleteOnClose);

	// Control types...
	m_ui.ControlTypeComboBox->clear();
	m_ui.ControlTypeComboBox->addItem(
		samplv1_controls::textFromType(samplv1_controls::CC),
		int(samplv1_controls::CC));
	m_ui.ControlTypeComboBox->addItem(
		samplv1_controls::textFromType(samplv1_controls::RPN),
		int(samplv1_controls::RPN));
	m_ui.ControlTypeComboBox->addItem(
		samplv1_controls::textFromType(samplv1_controls::NRPN),
		int(samplv1_controls::NRPN));
	m_ui.ControlTypeComboBox->addItem(
		samplv1_controls::textFromType(samplv1_controls::CC14),
		int(samplv1_controls::CC14));

	m_ui.ControlParamComboBox->setInsertPolicy(QComboBox::NoInsert);

	// Start clean.
	m_iControlParamUpdate = 0;

	m_iDirtyCount = 0;
	m_iDirtySetup = 0;

	// Populate param list.
	// activateControlType(m_ui.ControlTypeComboBox->currentIndex());

	// Try to fix window geometry.
	adjustSize();

	// UI signal/slot connections...
	QObject::connect(m_ui.ControlTypeComboBox,
		SIGNAL(activated(int)),
		SLOT(activateControlType(int)));
	QObject::connect(m_ui.ControlParamComboBox,
		SIGNAL(activated(int)),
		SLOT(changed()));
	QObject::connect(m_ui.ControlChannelSpinBox,
		SIGNAL(valueChanged(int)),
		SLOT(changed()));
	QObject::connect(m_ui.DialogButtonBox,
		SIGNAL(clicked(QAbstractButton *)),
		SLOT(clicked(QAbstractButton *)));
	QObject::connect(m_ui.DialogButtonBox,
		SIGNAL(accepted()),
		SLOT(accept()));
	QObject::connect(m_ui.DialogButtonBox,
		SIGNAL(rejected()),
		SLOT(reject()));

	// Pseudo-singleton reference setup.
	g_pInstance = this;
}


// Destructor.
samplv1widget_control::~samplv1widget_control (void)
{
}


// Pseudo-singleton instance.
samplv1widget_control *samplv1widget_control::getInstance (void)
{
	return g_pInstance;
}


// Pseudo-constructor.
void samplv1widget_control::showInstance (
	samplv1_controls *pControls, samplv1::ParamIndex index,
	QWidget *pParent, Qt::WindowFlags wflags )
{
	samplv1widget_control *pInstance = samplv1widget_control::getInstance();
	if (pInstance)
		pInstance->close();

	pInstance = new samplv1widget_control(pParent, wflags);
	pInstance->setControls(pControls, index);
	pInstance->show();
}


// Control accessors.
void samplv1widget_control::setControls (
	samplv1_controls *pControls, samplv1::ParamIndex index )
{
	m_pControls = pControls;
	m_index = index;

	const QString sParamName = samplv1_param::paramName(m_index);
	QDialog::setWindowTitle(sParamName + " - " + tr("MIDI Controller"));

	++m_iDirtySetup;

	setControlKey(controlKey());

	--m_iDirtySetup;

	m_iDirtyCount = 0;
}


samplv1_controls *samplv1widget_control::controls (void) const
{
	return m_pControls;
}


samplv1::ParamIndex samplv1widget_control::controlIndex (void) const
{
	return m_index;
}


// Pseudo-destructor.
void samplv1widget_control::closeEvent ( QCloseEvent *pCloseEvent )
{
	// Pseudo-singleton reference setup.
	g_pInstance = NULL;

	// Sure acceptance and probable destruction (cf. WA_DeleteOnClose).
	QDialog::closeEvent(pCloseEvent);
}


// Process incoming controller key event.
void samplv1widget_control::setControlKey ( const samplv1_controls::Key& key )
{
	setControlType(key.type());
	setControlParam(key.param);

	m_ui.ControlChannelSpinBox->setValue(key.channel());
}


samplv1_controls::Key samplv1widget_control::controlKey (void) const
{
	samplv1_controls::Key key;

	if (m_pControls) {
		const samplv1_controls::Map& map = m_pControls->map();
		samplv1_controls::Map::ConstIterator iter = map.constBegin();
		const samplv1_controls::Map::ConstIterator& iter_end
			= map.constEnd();
		for ( ; iter != iter_end; ++iter) {
			if (samplv1::ParamIndex(iter.value()) == m_index) {
				key = iter.key();
				break;
			}
		}
	}

	return key;
}


// Change settings (anything else slot).
void samplv1widget_control::changed (void)
{
	if (m_iDirtySetup > 0)
		return;

#ifdef CONFIG_DEBUG_0
	qDebug("samplv1widget_control::changed()");
#endif

	++m_iDirtyCount;

	stabilize();
}


// Accept settings (OK button slot).
void samplv1widget_control::accept (void)
{
	if (m_pControls == NULL)
		return;

#ifdef CONFIG_DEBUG_0
	qDebug("samplv1widget_control::accept()");
#endif

	// Get map settings...
	const samplv1_controls::Type ctype = controlType();
	const unsigned short channel = controlChannel();

	samplv1_controls::Key key;
	key.status = ctype | (channel & 0x1f);
	key.param = controlParam();

	// Check if already mapped to someone else...
	const int iIndex = m_pControls->find_control(key);
	if (iIndex >= 0 && samplv1::ParamIndex(iIndex) != m_index) {
		if (QMessageBox::warning(this,
			QDialog::windowTitle(),
			tr("MIDI controller is already assigned.\n\n"
			"Do you want to replace the mapping?"),
			QMessageBox::Ok |
			QMessageBox::Cancel) == QMessageBox::Cancel)
			return;
	}

	// Unmap the existing controller....
	if (iIndex >= 0)
		m_pControls->remove_control(key);

	// Map the damn controller....
	m_pControls->add_control(key, m_index);

	// Aint't dirty no more...
	m_iDirtyCount = 0;

	// Just go with dialog acceptance...
	QDialog::accept();
	QDialog::close();
}


// Reject settings (Cancel button slot).
void samplv1widget_control::reject (void)
{
#ifdef CONFIG_DEBUG_0
	qDebug("samplv1widget_control::reject()");
#endif

	// Check if there's any pending changes...
	if (m_iDirtyCount > 0) {
		switch (QMessageBox::warning(this,
			QDialog::windowTitle(),
			tr("Some settings have been changed.\n\n"
			"Do you want to apply the changes?"),
			QMessageBox::Apply |
			QMessageBox::Discard |
			QMessageBox::Cancel)) {
		case QMessageBox::Discard:
			break;
		case QMessageBox::Apply:
			accept();
		default:
			return;
		}
	}

	// Just go with dialog rejection...
	QDialog::reject();
	QDialog::close();
}


// Perotected slots.
void samplv1widget_control::activateControlType ( int iControlType )
{
#ifdef CONFIG_DEBUG_0
	qDebug("samplv1widget_control::activateControlType(%d)", iControlType);
#endif

	updateControlType(iControlType);

	changed();
}

void samplv1widget_control::editControlParamFinished (void)
{
	if (m_iControlParamUpdate > 0)
		return;

	++m_iControlParamUpdate;

	const QString& sControlParam
		= m_ui.ControlParamComboBox->currentText();

	bool bOk = false;
	sControlParam.toInt(&bOk);
	if (bOk) changed();

	--m_iControlParamUpdate;
}


void samplv1widget_control::stabilize (void)
{
	const bool bValid = (m_iDirtyCount > 0);
	m_ui.DialogButtonBox->button(QDialogButtonBox::Ok)->setEnabled(bValid);
}


// Control type dependency refresh.
void samplv1widget_control::updateControlType ( int iControlType )
{
	if (iControlType < 0)
		iControlType = m_ui.ControlTypeComboBox->currentIndex();

	const samplv1_controls::Type ctype
		= controlTypeFromIndex(iControlType);

	const bool bOldEditable
		= m_ui.ControlParamComboBox->isEditable();
	const int iOldParam
		= m_ui.ControlParamComboBox->currentIndex();
	const QString sOldParam
		= m_ui.ControlParamComboBox->currentText();

	m_ui.ControlParamComboBox->clear();

	const QString sMask("%1 - %2");
	switch (ctype) {
	case samplv1_controls::CC: {
		if (m_ui.ControlParamTextLabel)
			m_ui.ControlParamTextLabel->setEnabled(true);
		m_ui.ControlParamComboBox->setEnabled(true);
		m_ui.ControlParamComboBox->setEditable(false);
		const samplv1widget_controls::Names& controllers
			= samplv1widget_controls::controllerNames();
		for (unsigned short param = 0; param < 128; ++param) {
			m_ui.ControlParamComboBox->addItem(
				sMask.arg(param).arg(controllers.value(param)),
				int(param));
		}
		break;
	}
	case samplv1_controls::RPN: {
		if (m_ui.ControlParamTextLabel)
			m_ui.ControlParamTextLabel->setEnabled(true);
		m_ui.ControlParamComboBox->setEnabled(true);
		m_ui.ControlParamComboBox->setEditable(true);
		const samplv1widget_controls::Names& rpns
			= samplv1widget_controls::rpnNames();
		samplv1widget_controls::Names::ConstIterator iter = rpns.constBegin();
		const samplv1widget_controls::Names::ConstIterator& iter_end
			= rpns.constEnd();
		for ( ; iter != iter_end; ++iter) {
			const unsigned short param = iter.key();
			m_ui.ControlParamComboBox->addItem(
				sMask.arg(param).arg(iter.value()),
				int(param));
		}
		break;
	}
	case samplv1_controls::NRPN: {
		if (m_ui.ControlParamTextLabel)
			m_ui.ControlParamTextLabel->setEnabled(true);
		m_ui.ControlParamComboBox->setEnabled(true);
		m_ui.ControlParamComboBox->setEditable(true);
		const samplv1widget_controls::Names& nrpns
			= samplv1widget_controls::nrpnNames();
		samplv1widget_controls::Names::ConstIterator iter = nrpns.constBegin();
		const samplv1widget_controls::Names::ConstIterator& iter_end
			= nrpns.constEnd();
		for ( ; iter != iter_end; ++iter) {
			const unsigned short param = iter.key();
			m_ui.ControlParamComboBox->addItem(
				sMask.arg(param).arg(iter.value()),
				int(param));
		}
		break;
	}
	case samplv1_controls::CC14: {
		if (m_ui.ControlParamTextLabel)
			m_ui.ControlParamTextLabel->setEnabled(true);
		m_ui.ControlParamComboBox->setEnabled(true);
		m_ui.ControlParamComboBox->setEditable(false);
		const samplv1widget_controls::Names& control14s
			= samplv1widget_controls::control14Names();
		for (unsigned short param = 1; param < 32; ++param) {
			m_ui.ControlParamComboBox->addItem(
				sMask.arg(param).arg(control14s.value(param)),
				int(param));
		}
		break;
	}
	default:
		break;
	}

	if (iOldParam >= 0 && iOldParam < m_ui.ControlParamComboBox->count())
		m_ui.ControlParamComboBox->setCurrentIndex(iOldParam);

	if (m_ui.ControlParamComboBox->isEditable()) {
		QObject::connect(m_ui.ControlParamComboBox->lineEdit(),
			SIGNAL(editingFinished()),
			SLOT(editControlParamFinished()));
		if (bOldEditable)
			m_ui.ControlParamComboBox->setEditText(sOldParam);
	}
}


void samplv1widget_control::setControlType ( samplv1_controls::Type ctype )
{
	const int iControlType = indexFromControlType(ctype);
	m_ui.ControlTypeComboBox->setCurrentIndex(iControlType);
	updateControlType(iControlType);
}


samplv1_controls::Type samplv1widget_control::controlType (void) const
{
	return controlTypeFromIndex(m_ui.ControlTypeComboBox->currentIndex());
}


void samplv1widget_control::setControlParam ( unsigned short param )
{
	const int iControlParam = indexFromControlParam(param);
	if (iControlParam >= 0) {
		m_ui.ControlParamComboBox->setCurrentIndex(iControlParam);
	} else {
		const QString& sControlParam = QString::number(param);
		m_ui.ControlParamComboBox->setEditText(sControlParam);
	}
}


unsigned short samplv1widget_control::controlParam (void) const
{
	if (m_ui.ControlParamComboBox->isEditable()) {
		unsigned short param = 0;
		const QString& sControlParam
			= m_ui.ControlParamComboBox->currentText();
		bool bOk = false;
		param = sControlParam.toInt(&bOk);
		if (bOk) return param;
	}

	return controlParamFromIndex(m_ui.ControlParamComboBox->currentIndex());
}


unsigned short samplv1widget_control::controlChannel (void) const
{
	return m_ui.ControlChannelSpinBox->value();
}


samplv1_controls::Type samplv1widget_control::controlTypeFromIndex ( int iIndex ) const
{
	if (iIndex >= 0 && iIndex < m_ui.ControlTypeComboBox->count())
		return samplv1_controls::Type(
			m_ui.ControlTypeComboBox->itemData(iIndex).toInt());
	else
		return samplv1_controls::CC;
}


int samplv1widget_control::indexFromControlType ( samplv1_controls::Type ctype ) const
{
	return m_ui.ControlTypeComboBox->findData(int(ctype));
}


unsigned short samplv1widget_control::controlParamFromIndex ( int iIndex ) const
{
	if (iIndex >= 0 && iIndex < m_ui.ControlParamComboBox->count())
		return m_ui.ControlParamComboBox->itemData(iIndex).toInt();
	else
		return 0;
}


int samplv1widget_control::indexFromControlParam ( unsigned short param ) const
{
	return m_ui.ControlParamComboBox->findData(int(param));
}


// end of samplv1widget_control.cpp
