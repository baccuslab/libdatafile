/* oawindow.cpp
 * ------------
 *
 * Implementation of online analysis window.
 *
 * (C) 2015 Benjamin Naecker bnaecker@stanford.edu
 */

#include <numeric>
#include <QCoreApplication>
#include <QFileDialog>
#include <QDebug>
#include <QFrame>
#include "oawindow.h"

OAWindow::OAWindow(QWidget *parent) : QWidget(parent, Qt::Window)
{
	setWindowTitle("Online analysis");
	setGeometry(0, 0, OAWINDOW_WIDTH, OAWINDOW_HEIGHT);
	loadPlugins();
	if (analyses.empty()) {
		initEmpty();
		return;
	}
	analysis = analyses[0];
	initGui();
}

OAWindow::~OAWindow(void) { }

void OAWindow::initEmpty(void)
{
	layout = new QGridLayout(this);
	analysisDescription = new QLabel("No online analysis plugins found", this);
	layout->addWidget(analysisDescription);
	setLayout(layout);
}

void OAWindow::initGui(void)
{
	layout = new QGridLayout(this);
	analysisBox = new QComboBox(this);
	analysisBox->addItems(pluginNames);
	connect(analysisBox, SIGNAL(currentIndexChanged(int)),
			this, SLOT(setAnalysis(int)));
	channelLabel = new QLabel("Channel:", this);
	channelBox = new QSpinBox(this);
	channelBox->setRange(0, 63);
	channelBox->setValue(0);
	instructions = new QLabel("Select analysis:", this);
	analysisDescription = new QLabel(analysis->description(), this);
	analysisDescription->setWordWrap(true);
	analysisDescription->setFrameStyle(QFrame::Box);
	runButton = new QPushButton("Run", this);
	runButton->setCheckable(true);
	runButton->setToolTip("Run selected analysis");
	runButton->setEnabled(false);
	connect(runButton, &QPushButton::toggled, 
			this, &OAWindow::toggleAnalysis);
	stimLabel = new QLabel("Stimulus:", this);
	stimLine = new QLineEdit(stimFile, this);
	stimLine->setReadOnly(true);
	chooseStimButton = new QPushButton("Choose", this);
	chooseStimButton->setToolTip("Choose a stimulus file for online analysis");
	connect(chooseStimButton, &QPushButton::clicked,
			this, &OAWindow::chooseStimulusFile);

	initPlot();

	layout->addWidget(instructions, 0, 0);
	layout->addWidget(analysisBox, 0, 1, 1, 2);
	layout->addWidget(runButton, 0, 3);
	layout->addWidget(channelLabel, 1, 0);
	layout->addWidget(channelBox, 1, 1);
	layout->addWidget(stimLabel, 2, 0);
	layout->addWidget(stimLine, 2, 1, 1, 2);
	layout->addWidget(chooseStimButton, 2, 3);
	layout->addWidget(analysisDescription, 3, 0, 1, 4);
	layout->addWidget(plot, 4, 0, 4, 4);
	setLayout(layout);
}

void OAWindow::initPlot(void)
{
	plot = new QCustomPlot(this);
}

void OAWindow::loadPlugins(void) 
{
	pluginDir = QDir(qApp->applicationDirPath());
#if defined(Q_OS_MAC)
	if (pluginDir.dirName() == "MacOS") {
		pluginDir.cdUp();
		pluginDir.cdUp();
		pluginDir.cdUp();
	}
#endif
	pluginDir.cd(PLUGIN_PATH);
	for (auto& file : pluginDir.entryList(QDir::Files)) {
		QPluginLoader loader(pluginDir.absoluteFilePath(file));
		QObject *plugin = loader.instance();
		if (plugin)
			addPlugin(plugin);
	}
}

void OAWindow::addPlugin(QObject *plugin)
{
	OAInterface *iface = qobject_cast<OAInterface *>(plugin);
	if (iface) {
		iface->init();
		analyses << iface;
		pluginNames << iface->name();
	}
}

void OAWindow::toggleVisible(void)
{
	setVisible(!isVisible());
}

void OAWindow::runAnalysis(uint64_t start, double rate, arma::vec d)
{
	/* Compute frames to grab from stimulus */
	unsigned int firstFrame = stimulus->frameBefore(start / rate);
	unsigned int lastFrame = stimulus->frameBefore((start + d.n_elem) / rate);
	unsigned int factor = rate / stimulus->nominalRate();

	if (stimulus->ndim() == 1) {
		arma::vec tmp, out;
		analysis->run(start, rate, d, stimulus, tmp);
		analysis->get(out);

		/*
		arma::vec stim, out;
		stimulus->frames(firstFrame, lastFrame, stim);
		//stim = arma::vectorise(arma::ones(factor, 1) * stim.t());
		analysis->run(d, stim, out);
		*/

		QVector<double> x(out.n_elem), y(out.n_elem);
		std::iota(x.begin(), x.end(), 0);
		for (auto i = 0; i < out.n_elem; i++)
			y[i] = out(i);
		//std::reverse(x.begin(), x.end());
		//a += out;
		//for (auto i = 0; i < a.n_elem; i++)
			//y[i] = a(i);
		plot->graph(0)->setData(x, y);
		plot->rescaleAxes();
		plot->replot();
	} else if (stimulus->ndim() == 2) {
		/*
		arma::mat stim, out;
		stimulus->frames(firstFrame, lastFrame, stim);
		analysis->run(d, stim, out);
		QCPColorMap *m = qobject_cast<QCPColorMap *>(plot->plottable(0));
		for (auto i = 0; i < out.n_rows; i++) {
			for (auto j = 0; j < out.n_cols; j++)
				m->data()->setCell(i, j, out(i, j));
		}
		m->rescaleDataRange();
		plot->replot();
		*/
	} else {
		/*
		arma::cube s, out;
		stimulus->frames(firstFrame, lastFrame, s);
		arma::cube stim(s.n_rows, s.n_cols, d.n_elem);
		for (auto i = 0; i < s.n_slices; i++) {
			for (auto j = 0; j < factor; j++)
				stim.slice(factor * i + j) = s.slice(i);
		}
		analysis->run(d, stim, out);
		QVector<double> x(out.n_slices), y(out.n_slices);
		std::iota(x.begin(), x.end(), 0);
		for (auto i = 0; i < out.n_slices; i++)
			y[i] = out(0, 0, i);
		plot->graph(0)->setData(x, y);
		QCPColorMap *m = qobject_cast<QCPColorMap *>(plot->plottable(1));
		for (auto i = 0; i < out.n_rows; i++) {
			for (auto j = 0; j < out.n_cols; j++) {
				m->data()->setCell(i, j, out(i, j, 0));
			}
		}
		plot->rescaleAxes();
		plot->replot();
		*/
	}
}

void OAWindow::toggleAnalysis(void)
{
	emit setRunning(!running, channel);
	running = !running;
}

void OAWindow::chooseStimulusFile(void)
{
	stimFile = QFileDialog::getOpenFileName(
			this, "Choose stimulus file", QDir::homePath(),
			"Stimuli (*.h5)");
	if (stimFile.isNull())
		return;
	try {
		stimulus = new Stimulus(stimFile.toStdString());
	} catch (std::invalid_argument &e) {
		QMessageBox::critical(this, "Invalid stimulus file",
				"Stimulus file '" + stimFile + "' is not a valid HDF5 file, " +
				"or does not contain a stimulus dataset");
		return;
	}
	if (!checkDimensionMatch())
		return;
	stimLine->setText(stimFile);
	runButton->setEnabled(true);
	setupPlot();
}

void OAWindow::setupPlot(void)
{
	plot->clearItems();
	plot->clearGraphs();
	plot->plotLayout()->removeAt(0);

	if (stimulus->ndim() == 1) {
		plot->plotLayout()->expandTo(1, 1);	
		QCPAxisRect *rect = new QCPAxisRect(plot);
		plot->plotLayout()->addElement(0, 0, rect);
		QCPGraph *g = new QCPGraph(
				rect->axis(QCPAxis::atBottom), rect->axis(QCPAxis::atLeft));
		plot->addPlottable(g);

		g->setPen(QPen(Qt::black, 3, Qt::SolidLine));
		g->keyAxis()->grid()->setPen(Qt::NoPen);
		g->keyAxis()->grid()->setZeroLinePen(QPen(Qt::black, 1, Qt::DashLine));
		g->keyAxis()->setRange(0, analysis->npoints());
		g->keyAxis()->setLabel("Time");
		g->keyAxis()->setLabelFont(QFont("Helvetica", 12, QFont::Light));
		//g->valueAxis()->setTicks(false);
		//g->valueAxis()->setTickLabels(false);
		g->valueAxis()->grid()->setPen(Qt::NoPen);
		g->valueAxis()->grid()->setZeroLinePen(QPen(Qt::black, 1, Qt::DashLine));
	} else if (stimulus->ndim() == 2) {
		plot->plotLayout()->expandTo(1, 1);
		QCPAxisRect *rect = new QCPAxisRect(plot);
		plot->plotLayout()->addElement(0, 0, rect);
		QCPColorMap *m = new QCPColorMap(
				rect->axis(QCPAxis::atBottom), rect->axis(QCPAxis::atLeft));
		m->setInterpolate(false);
		m->data()->setSize(stimulus->nx(), analysis->npoints());
		m->data()->setRange(QCPRange(0, stimulus->nx()), QCPRange(0, analysis->npoints()));
		m->setGradient(QCPColorGradient::gpGrayscale);
		plot->addPlottable(m);

		m->keyAxis()->grid()->setPen(Qt::NoPen);
		m->keyAxis()->setRange(0, analysis->npoints());
		m->keyAxis()->setLabel("Time");
		m->keyAxis()->setLabelFont(QFont("Helvetica", 12, QFont::Light));
		m->valueAxis()->setRange(0, stimulus->nx());
		m->valueAxis()->setTicks(false);
		m->valueAxis()->setTickLabels(false);
		m->valueAxis()->grid()->setPen(Qt::NoPen);
		m->valueAxis()->setLabel("Space");
		m->valueAxis()->setLabelFont(QFont("Helvetica", 12, QFont::Light));
		m->valueAxis()->grid()->setZeroLinePen(QPen(Qt::black, 1, Qt::DashLine));
	} else {

		plot->plotLayout()->expandTo(1, 2);
		QCPAxisRect *graphRect = new QCPAxisRect(plot);
		plot->plotLayout()->addElement(0, 0, graphRect);
		QCPGraph *g = new QCPGraph(
				graphRect->axis(QCPAxis::atBottom), graphRect->axis(QCPAxis::atLeft));
		plot->addPlottable(g);

		g->setPen(QPen(Qt::black, 3, Qt::SolidLine));
		g->keyAxis()->grid()->setPen(Qt::NoPen);
		g->keyAxis()->grid()->setZeroLinePen(QPen(Qt::black, 1, Qt::DashLine));
		g->keyAxis()->setRange(0, analysis->npoints());
		g->keyAxis()->setLabel("Time");
		g->keyAxis()->setLabelFont(QFont("Helvetica", 12, QFont::Light));
		g->valueAxis()->setTicks(false);
		g->valueAxis()->setTickLabels(false);
		g->valueAxis()->grid()->setPen(Qt::NoPen);
		g->valueAxis()->grid()->setZeroLinePen(QPen(Qt::black, 1, Qt::DashLine));


		QCPAxisRect *mapRect = new QCPAxisRect(plot);
		QCPColorMap *m = new QCPColorMap(
				mapRect->axis(QCPAxis::atBottom), mapRect->axis(QCPAxis::atLeft));
		plot->plotLayout()->addElement(0, 1, mapRect);
		m->setInterpolate(false);
		m->data()->setSize(stimulus->nx(), stimulus->ny());
		m->data()->setRange(QCPRange(0, stimulus->nx()), QCPRange(0, stimulus->ny()));
		m->setGradient(QCPColorGradient::gpGrayscale);
		plot->addPlottable(m);

		m->keyAxis()->grid()->setPen(Qt::NoPen);
		m->keyAxis()->setRange(0, stimulus->nx());
		m->keyAxis()->setLabel("X");
		m->keyAxis()->setLabelFont(QFont("Helvetica", 12, QFont::Light));
		m->valueAxis()->setRange(0, stimulus->ny());
		m->valueAxis()->setTicks(false);
		m->valueAxis()->setTickLabels(false);
		m->valueAxis()->grid()->setPen(Qt::NoPen);
		m->valueAxis()->setLabel("Y");
		m->valueAxis()->setLabelFont(QFont("Helvetica", 12, QFont::Light));
		m->valueAxis()->grid()->setZeroLinePen(QPen(Qt::black, 1, Qt::DashLine));
	}
	plot->replot();
}

void OAWindow::setAnalysis(int index)
{
	analysis = analyses[index];
	if (!checkDimensionMatch())
		return;
	setupPlot();
	analysisDescription->setText(analysis->description());
}

bool OAWindow::checkDimensionMatch(void)
{
	if (analysis->ndim() != stimulus->ndim()) {
		return (QMessageBox::warning(
				this, "Dimension mismatch",
				"The selected analysis and stimulus have different numbers of dimensions",
				QMessageBox::Ok | QMessageBox::Cancel) == QMessageBox::Ok);
	}
	return true;
}
