/* settings.cpp
 * Implemenation file for various settings and the Settings class.
 * (C) 2015 Benjamin Naecker bnaecker@stanford.edu.
 */

#include "settings.h"

Settings::Settings() {
}

Settings::~Settings() {
}

QPen Settings::getPlotPen() {
	return qvariant_cast<QPen>(settings.value("plot-pen"));
}

void Settings::setPlotPen(QPen p) {
	settings.setValue("plot-pen", QVariant(p));
}

float Settings::getDisplayRange() {
	return settings.value("display-range").toFloat();
}

float Settings::getDisplayScale() {
	return settings.value("display-scale").toFloat();
}

void Settings::setDisplayScale(float scale) {
	settings.setValue("display-scale", QVariant(scale));
}

QList<float> Settings::getDisplayScales() {
	return DISPLAY_SCALES;
}

float Settings::getRefreshInterval() {
	return settings.value("refresh-interval").toFloat();
}

void Settings::setRefreshInterval(float interval) {
	float i = qMin(qMax(interval, MIN_REFRESH_INTERVAL), MAX_REFRESH_INTERVAL);
	settings.setValue("refresh-interval", QVariant(i));
}

QList<QPair<int, int> > Settings::getChannelView() {
	return CHANNEL_VIEW_MAP[getChannelViewString()];
}

QString Settings::getChannelViewString() {
	return settings.value("channel-view-string").toString();
}

void Settings::setChannelView(QString s) {
	settings.setValue("channel-view-string", QVariant(s));
}

QString Settings::getSaveDir() {
	return settings.value("save-dir").toString();
}

void Settings::setSaveDir(QString s) {
	settings.setValue("save-dir", QVariant(s));
}

QString Settings::getSaveFilename() {
	return settings.value("save-filename").toString();
}

void Settings::setSaveFilename(QString s) {
	settings.setValue("save-filename", QVariant(s));
}

unsigned int Settings::getExperimentLength() {
	return settings.value("experiment-length").toUInt();
}

void Settings::setExperimentLength(unsigned int len) {
	settings.setValue("experiment-length", QVariant(len));
}

bool Settings::getAutoscale() {
	return settings.value("autoscale").toBool();
}

void Settings::setAutoscale(bool on) {
	settings.setValue("autoscale", QVariant(on));
}

unsigned int Settings::getOnlineAnalysisLength() {
	return settings.value("online-analysis-length").toUInt();
}

void Settings::setOnlineAnalysisLength(unsigned int len) {
	unsigned int val = qMax(qMin(len, ONLINE_ANALYSIS_MAX_LENGTH), 
			ONLINE_ANALYSIS_MIN_LENGTH);
	settings.setValue("online-analysis-length", QVariant(val));
}

unsigned int Settings::getJump() {
	return settings.value("jump").toUInt();
}

void Settings::setJump(unsigned int jump) {
	unsigned int val = qMax(qMin(jump, JUMP_MAX), JUMP_MIN);
	settings.setValue("jump", QVariant(val));
}

int Settings::getNumRows() {
	return settings.value("num-rows").toInt();
}

int Settings::getNumCols() {
	return settings.value("num-cols").toInt();
}

void Settings::setNumRows(int rows) {
	settings.setValue("num-rows", QVariant(rows));
}

void Settings::setNumCols(int cols) {
	settings.setValue("num-cols", QVariant(cols));
}

bool Settings::getAutoMean() {
	return settings.value("automean").toBool();
}

void Settings::setAutoMean(bool b) {
	settings.setValue("automean", QVariant(b));
}

