// samplv1_param.cpp
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

#include "samplv1_param.h"
#include "samplv1_config.h"

#include <QHash>

#include <QDomDocument>
#include <QTextStream>
#include <QDir>


//-------------------------------------------------------------------------
// default state (params)

static
struct {

	const char *name;
	float value;

} samplv1_default_params[samplv1::NUM_PARAMS] = {

	{ "GEN1_SAMPLE",   60.0f }, // middle-C aka. C4 (60)
	{ "GEN1_REVERSE",   0.0f },
	{ "GEN1_LOOP",      0.0f },
	{ "GEN1_OCTAVE",    0.0f },
	{ "GEN1_TUNING",    0.0f },
	{ "GEN1_GLIDE",     0.0f },
	{ "GEN1_ENVTIME",   0.5f },
	{ "DCF1_CUTOFF",    1.0f }, // 0.5f
	{ "DCF1_RESO",      0.0f },
	{ "DCF1_TYPE",      0.0f },
	{ "DCF1_SLOPE",     0.0f },
	{ "DCF1_ENVELOPE",  1.0f },
	{ "DCF1_ATTACK",    0.0f },
	{ "DCF1_DECAY",     0.2f },
	{ "DCF1_SUSTAIN",   0.5f },
	{ "DCF1_RELEASE",   0.5f },
	{ "LFO1_SHAPE",     1.0f },
	{ "LFO1_WIDTH",     1.0f },
	{ "LFO1_RATE",      0.5f },
	{ "LFO1_SWEEP",     0.0f },
	{ "LFO1_PITCH",     0.0f },
	{ "LFO1_CUTOFF",    0.0f },
	{ "LFO1_RESO",      0.0f },
	{ "LFO1_PANNING",   0.0f },
	{ "LFO1_VOLUME",    0.0f },
	{ "LFO1_ATTACK",    0.0f },
	{ "LFO1_DECAY",     0.1f },
	{ "LFO1_SUSTAIN",   1.0f },
	{ "LFO1_RELEASE",   0.5f },
	{ "DCA1_VOLUME",    0.5f },
	{ "DCA1_ATTACK",    0.0f },
	{ "DCA1_DECAY",     0.1f },
	{ "DCA1_SUSTAIN",   1.0f },
	{ "DCA1_RELEASE",   0.5f },	// 0.1f
	{ "OUT1_WIDTH",     0.0f },
	{ "OUT1_PANNING",   0.0f },
	{ "OUT1_VOLUME",    0.5f },

	{ "DEF1_PITCHBEND", 0.2f },
	{ "DEF1_MODWHEEL",  0.2f },
	{ "DEF1_PRESSURE",  0.2f },
	{ "DEF1_VELOCITY",  0.2f },
	{ "DEF1_CHANNEL",   0.0f },
	{ "DEF1_MONO",      0.0f },

	{ "CHO1_WET",       0.0f },
	{ "CHO1_DELAY",     0.5f },
	{ "CHO1_FEEDB",     0.5f },
	{ "CHO1_RATE",      0.5f },
	{ "CHO1_MOD",       0.5f },
	{ "FLA1_WET",       0.0f },
	{ "FLA1_DELAY",     0.5f },
	{ "FLA1_FEEDB",     0.5f },
	{ "FLA1_DAFT",      0.0f },
	{ "PHA1_WET",       0.0f },
	{ "PHA1_RATE",      0.5f },
	{ "PHA1_FEEDB",     0.5f },
	{ "PHA1_DEPTH",     0.5f },
	{ "PHA1_DAFT",      0.0f },
	{ "DEL1_WET",       0.0f },
	{ "DEL1_DELAY",     0.5f },
	{ "DEL1_FEEDB",     0.5f },
	{ "DEL1_BPM",     180.0f },
	{ "DEL1_BPMSYNC",   0.0f },
	{ "DEL1_BPMHOST", 180.0f },
	{ "REV1_WET",       0.0f },
	{ "REV1_ROOM",      0.5f },
	{ "REV1_DAMP",      0.5f },
	{ "REV1_FEEDB",     0.5f },
	{ "REV1_WIDTH",     0.0f },
	{ "DYN1_COMPRESS",  0.0f },
	{ "DYN1_LIMITER",   1.0f }
};


const char *samplv1_param::paramName ( samplv1::ParamIndex index )
{
	return samplv1_default_params[index].name;
}


float samplv1_param::paramDefaultValue ( samplv1::ParamIndex index )
{
	return samplv1_default_params[index].value;
}


// Sample serialization methods.
void samplv1_param::loadSamples (
	samplv1_ui *pSamplUi, const QDomElement& eSamples )
{
	if (pSamplUi == NULL)
		return;

	for (QDomNode nSample = eSamples.firstChild();
			!nSample.isNull();
				nSample = nSample.nextSibling()) {
		QDomElement eSample = nSample.toElement();
		if (eSample.isNull())
			continue;
		if (eSample.tagName() == "sample") {
		//	int index = eSample.attribute("index").toInt();
			QString sFilename;
			uint32_t iLoopStart = 0;
			uint32_t iLoopEnd = 0;
			for (QDomNode nChild = eSample.firstChild();
					!nChild.isNull();
						nChild = nChild.nextSibling()) {
				QDomElement eChild = nChild.toElement();
				if (eChild.isNull())
					continue;
				if (eChild.tagName() == "filename") {
				//	int index = eSample.attribute("index").toInt();
					sFilename = eChild.text();
				}
				else
				if (eChild.tagName() == "loop-start") {
					iLoopStart = eChild.text().toULong();
				}
				else
				if (eChild.tagName() == "loop-end") {
					iLoopEnd = eChild.text().toULong();
				}
			}
			// Legacy loader...
			if (sFilename.isEmpty())
				sFilename = eSample.text();
			// Done it.
			pSamplUi->setSampleFile(sFilename.toUtf8().constData());
			// Set actual sample loop points...
			pSamplUi->setLoopRange(iLoopStart, iLoopEnd);
		}
	}
}


void samplv1_param::saveSamples (
	samplv1_ui *pSamplUi, QDomDocument& doc, QDomElement& eSamples )
{
	if (pSamplUi == NULL)
		return;

	const char *pszSampleFile = pSamplUi->sampleFile();
	if (pszSampleFile == NULL)
		return;

	QDomElement eSample = doc.createElement("sample");
	eSample.setAttribute("index", 0);
	eSample.setAttribute("name", "GEN1_SAMPLE");

	QDomElement eFilename = doc.createElement("filename");
	eFilename.appendChild(doc.createTextNode(
		QDir::current().relativeFilePath(
			QString::fromUtf8(pszSampleFile))));
	eSample.appendChild(eFilename);

	const uint32_t iLoopStart = pSamplUi->loopStart();
	const uint32_t iLoopEnd   = pSamplUi->loopEnd();
	if (iLoopStart < iLoopEnd) {
		QDomElement eLoopStart = doc.createElement("loop-start");
		eLoopStart.appendChild(doc.createTextNode(
			QString::number(iLoopStart)));
		eSample.appendChild(eLoopStart);
		QDomElement eLoopEnd = doc.createElement("loop-end");
		eLoopEnd.appendChild(doc.createTextNode(
			QString::number(iLoopEnd)));
		eSample.appendChild(eLoopEnd);
	}

	eSamples.appendChild(eSample);
}


// Preset serialization methods.
void samplv1_param::loadPreset ( samplv1_ui *pSamplUi, const QString& sFilename )
{
	if (pSamplUi == NULL)
		return;

	QFileInfo fi(sFilename);
	if (!fi.exists()) {
		samplv1_config *pConfig = samplv1_config::getInstance();
		if (pConfig) {
			const QString& sPresetFile
				= pConfig->presetFile(sFilename);
			if (sPresetFile.isEmpty())
				return;
			fi.setFile(sPresetFile);
			if (!fi.exists())
				return;
		}
	}

	QFile file(fi.filePath());
	if (!file.open(QIODevice::ReadOnly))
		return;

	static QHash<QString, samplv1::ParamIndex> s_hash;
	if (s_hash.isEmpty()) {
		for (uint32_t i = 0; i < samplv1::NUM_PARAMS; ++i) {
			samplv1::ParamIndex index = samplv1::ParamIndex(i);
			s_hash.insert(samplv1_param::paramName(index), index);
		}
	}

	const QDir currentDir(QDir::current());
	QDir::setCurrent(fi.absolutePath());

	QDomDocument doc(SAMPLV1_TITLE);
	if (doc.setContent(&file)) {
		QDomElement ePreset = doc.documentElement();
		if (ePreset.tagName() == "preset"
			&& ePreset.attribute("name") == fi.completeBaseName()) {
			for (QDomNode nChild = ePreset.firstChild();
					!nChild.isNull();
						nChild = nChild.nextSibling()) {
				QDomElement eChild = nChild.toElement();
				if (eChild.isNull())
					continue;
				if (eChild.tagName() == "samples") {
					samplv1_param::loadSamples(pSamplUi, eChild);
				}
				else
				if (eChild.tagName() == "params") {
					for (QDomNode nParam = eChild.firstChild();
							!nParam.isNull();
								nParam = nParam.nextSibling()) {
						QDomElement eParam = nParam.toElement();
						if (eParam.isNull())
							continue;
						if (eParam.tagName() == "param") {
							samplv1::ParamIndex index = samplv1::ParamIndex(
								eParam.attribute("index").toULong());
							const QString& sName = eParam.attribute("name");
							if (!sName.isEmpty()) {
								if (!s_hash.contains(sName))
									continue;
								index = s_hash.value(sName);
							}
							float fValue = eParam.text().toFloat();
						#if 1//--legacy support < 0.3.0.4
							if (index == samplv1::DEL1_BPM && fValue < 3.6f)
								fValue *= 100.0f;
						#endif
							pSamplUi->setParamValue(index, fValue);
						}
					}
				}
			}
		}
	}

	file.close();

	pSamplUi->reset();

	QDir::setCurrent(currentDir.absolutePath());
}


void samplv1_param::savePreset ( samplv1_ui *pSamplUi, const QString& sFilename )
{
	if (pSamplUi == NULL)
		return;

	const QFileInfo fi(sFilename);
	const QDir currentDir(QDir::current());
	QDir::setCurrent(fi.absolutePath());

	QDomDocument doc(SAMPLV1_TITLE);
	QDomElement ePreset = doc.createElement("preset");
	ePreset.setAttribute("name", fi.completeBaseName());
	ePreset.setAttribute("version", SAMPLV1_VERSION);

	QDomElement eSamples = doc.createElement("samples");
	samplv1_param::saveSamples(pSamplUi, doc, eSamples);
	ePreset.appendChild(eSamples);

	QDomElement eParams = doc.createElement("params");
	for (uint32_t i = 0; i < samplv1::NUM_PARAMS; ++i) {
		QDomElement eParam = doc.createElement("param");
		samplv1::ParamIndex index = samplv1::ParamIndex(i);
		eParam.setAttribute("index", QString::number(i));
		eParam.setAttribute("name", samplv1_param::paramName(index));
		const float fValue = pSamplUi->paramValue(index);
		eParam.appendChild(doc.createTextNode(QString::number(fValue)));
		eParams.appendChild(eParam);
	}
	ePreset.appendChild(eParams);
	doc.appendChild(ePreset);

	QFile file(fi.filePath());
	if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
		QTextStream(&file) << doc.toString();
		file.close();
	}

	QDir::setCurrent(currentDir.absolutePath());
}


// end of samplv1_param.cpp
