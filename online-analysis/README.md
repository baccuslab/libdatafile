Mearec online analysis library
==============================

A library for performing online analysis of array or intracellular recordings.

(C) 2015 Benjamin Naecker bnaecker@stanford.edu

Overview
========

The Mearec online analysis library provides several common analyses, such as
spike-triggered averages, as well as a relatively easy API for writing new,
custom analyses. Stimuli must be created beforehand, and stored in HDF5 
files (see format below). The library provides tools for reading these stimuli
from disk, and an easy way to plug in custom functions to the analysis pipeline.

Stimulus files
==============

Stimuli are expected to be stored in HDF5 file formats. The stimulus itself
must be a single dataset, which can be 1-, 2-, or 3-dimensional, of arbitrary
size. It is expected that the last dimension is time. So, for example, 
a white noise lines stimulus would have dimension [nbars-by-nframes]. 

The dataset is assumed to be called simply "stimulus", but you can provide
a name of your choosing when constructing objects needed to represent the 
stimulus in memory. This allows you to perform analyses on only one of several
datasets in a single file.

The only other requirement of the data file is that the stimulus dataset must
have an attribute giving the expected frame rate of the monitor. This is used
to determine which frames of the stimulus will be read when performing analyses.

Analysis plugin API
===================

New online analyses can be written at any time, compiled, and added dynamically
to Mearec. Under the hood, this is done with Qt's plugin and dynamic library
functionality. As a user, this complexity is hidden; simply subclass the base
online analysis class and reimplement its `run` method. This method will be
called periodically, as new data is received, and implements the core computation
performed by the online analysis.

The `run` method is quite simple. It takes a chunk of data and a chunk of stimulus,
and performs some computation including the two. The amount of data and stimulus
that are received are defined by the refresh rate of the Mearec display. A faster
refresh rate means a shorter section of data and stimulus are handed off.

The library automatically handles upsampling of the stimulus array, based on the
monitor frame rate given to the analysis class constructors. The data is always
a 1-dimensional vector, whose number of elements matches the size of the last
dimension of the stimulus. This makes it easy to do reverse-correlation, for
example, by simply computing the convolution of the stimulus and data, and taking
a small chunk of the result.

There are three provided signatures for the `run` method, one for each possible
stimulus dimensionality.

	run(const arma::vec& data, const arma::vec& stim1d);
	run(const arma::vec& data, const arma::mat& stim2d);
	run(const arma::vec& data, const arma::cube& stim3d);


