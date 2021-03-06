#ifndef INTENSITYVOLUMEFEATURES_H_INCLUDED
#define INTENSITYVOLUMEFEATURES_H_INCLUDED

/*! \file */
/*!
Intensity volume histogram features describe the relationship between the grey level i and the volume fraction which contains
at least grey level i or higher.\n
For this, the intensity histogram and the corresponding volume fractions have to be calculated. \n
If the VOI contains only one grey level, the intensity volume features are not calculated. 
*/

#include <iostream>
#include <vector>
#include "boost/multi_array.hpp"
#include "boost/range/combine.hpp"
#include "boost/foreach.hpp"
#include "image.h"
using namespace std;

template <class T,  size_t R>
class IntensityVolumeFeatures{
    private:
        vector<T> diffGreyLevels;
        T maxGreyLevel;
        T minGreyLevel;
        vector<T> greyLevelFraction;
        vector<T> fracVolume; 
		vector<T> intensityVector;

        T volAtIntFrac10 = NAN;
        T volAtIntFrac90 = NAN;
        T intAtVolFrac10 = NAN;
        T intAtVolFrac90 = NAN;

        T diffVolAtIntFrac = NAN;
        T diffIntAtVolFrac = NAN;

        T getVolumeAtIntFraction(double percent);
        T getIntAtVolFraction(double percent, vector<T> diffGreyLevels);
        void defineIntVolFeatures(vector<string> &features);
		void defineIntVolFeaturesOntology(vector<string> &features);
        void extractIntVolData(vector<T> &intVolData, IntensityVolumeFeatures<T, R> intVolFeatures);

    public:
        IntensityVolumeFeatures(){

        }
		~IntensityVolumeFeatures() {
			
		}
        void getFractionalVolume(boost::multi_array<T,R> inputMatrix, vector<T> vectorMatrElem);
        void getGreyLevelFraction(boost::multi_array<T,R> inputMatrix);
        void calculateAllIntensVolFeatures(IntensityVolumeFeatures<T,R> &intVolFeatures, boost::multi_array<T, R> inputMatrix, vector<T> vectorMatrElem);
        void writeCSVFileIntVol(IntensityVolumeFeatures<T,R> intVol, string outputFolder);
		void writeOneFileIntVol(IntensityVolumeFeatures<T, R> intVol, ConfigFile config);
};


/*!
In the function getFractionalVolume the fractional volume of each grey level is calculated. \n
The vector fractional volume vector is filled in this function
@parameter[in]: boost multi_array input matrix: matrix containing intensity values of VOI
@parameter[in] vectorMatrElemen: vector containing all grey levels of VOI
*/
template <class T, size_t R>
void IntensityVolumeFeatures<T, R>::getFractionalVolume(boost::multi_array<T, R> inputMatrix, vector<T> vectorMatrElem) {
	T actFracVolume;
	double nrElementsSmaller;
	double nrElementsNotNAN;

	for (int greyLevel = minGreyLevel; greyLevel < maxGreyLevel + 1; greyLevel++) {
		nrElementsSmaller = 0;
		nrElementsNotNAN = 0;
		for (int depth = 0; depth < inputMatrix.shape()[2]; depth++) {
			for (int rows = 0; rows < inputMatrix.shape()[0]; rows++) {
				for (int col = 0; col < inputMatrix.shape()[1]; col++) {
					if (inputMatrix[rows][col][depth] < greyLevel && !isnan(inputMatrix[rows][col][depth])) {
						//std::cout << inputMatrix[rows][col][depth] << std::endl;
						nrElementsSmaller += 1;
					}
					if (!isnan(inputMatrix[rows][col][depth])) {
						nrElementsNotNAN += 1;
					}
				}
			}
		}
		actFracVolume = 1 - nrElementsSmaller / nrElementsNotNAN;
		fracVolume.push_back(actFracVolume);
	}

}

/*!
In the function getGreyLevelFraction the grey level fraction is calculated and appended to the vector greyLevelFraction. \n
@parameter[in]: boost multi_array input matrix: matrix containing intensity values of VOI
@parameter[in] vectorMatrElemen: vector containing all grey levels of VOI
*/
template <class T, size_t R>
void IntensityVolumeFeatures<T, R>::getGreyLevelFraction(boost::multi_array<T, R> inputMatrix) {
	T actGreyLevelFraction;

	for (int actGreyLevel = minGreyLevel; actGreyLevel < maxGreyLevel + 1; actGreyLevel++) {
		actGreyLevelFraction = (actGreyLevel - minGreyLevel) / (maxGreyLevel - minGreyLevel);
		greyLevelFraction.push_back(actGreyLevelFraction);
	}

}

/*!
In the function getVolumeAtIntFraction calculates the volume at a certain intensity fraction for a certain percentage value.
@parameter[in] double percent: percentage value for which the volume fraction is calculated
*/
template <class T, size_t R>
T IntensityVolumeFeatures<T, R>::getVolumeAtIntFraction(double percent) {
	vector<T> tempVector = greyLevelFraction;
	typename vector<T>::iterator it;
	typename vector<T>::iterator greaterThan;
	greaterThan = remove_if(tempVector.begin(), tempVector.end(), bind2nd(less<T>(), percent));
	tempVector.erase(greaterThan, tempVector.end());
	it = find(greyLevelFraction.begin(), greyLevelFraction.end(), tempVector[0]);
	int pos = std::distance(greyLevelFraction.begin(), it);
	return fracVolume[pos];
}


/*!
In the function getIntAtVolFraction calculates the intensity at a certain volume fraction for a certain percentage value.
@parameter[in] double percent: percentage value for which the volume fraction is calculated
*/
template <class T, size_t R>
T IntensityVolumeFeatures<T, R>::getIntAtVolFraction(double percent, vector<T> diffGreyLevels) {
	vector<T> tempVector = fracVolume;
	typename vector<T>::iterator it;
	typename vector<T>::iterator greaterThan;

	int pos;
	if (tempVector[boost::size(tempVector) - 1] < percent) {
		greaterThan = remove_if(tempVector.begin(), tempVector.end(), bind2nd(greater<T>(), percent));
		tempVector.erase(greaterThan, tempVector.end());
		it = find(fracVolume.begin(), fracVolume.end(), tempVector[0]);
		pos = std::distance(fracVolume.begin(), it);

	}
	else {
		std::cout << "The frac volume is never smaller than 90 percent, error in intensity at volume fraction calculation" << std::endl;
		pos = 0;
	}
	return minGreyLevel + pos;

}

template <class T, size_t R>
void IntensityVolumeFeatures<T, R>::calculateAllIntensVolFeatures(IntensityVolumeFeatures<T,R> &intVolFeatures, boost::multi_array<T, R> inputMatrix, vector<T> diffGreyLevels){
	
	vector<T> elementVector;
	for (int depth = 0; depth < inputMatrix.shape()[2]; depth++) {
		for (int row = 0; row < inputMatrix.shape()[0]; row++) {
			for (int col = 0; col < inputMatrix.shape()[1]; col++) {
				if (!std::isnan(inputMatrix[row][col][depth])) {
					elementVector.push_back(inputMatrix[row][col][depth]);
				}
			}
		}
	}
	maxGreyLevel = *max_element(elementVector.begin(), elementVector.end());
	minGreyLevel = *min_element(elementVector.begin(), elementVector.end());
	vector<T> temGreyLevels;
	//get Grey Levels which are not NAN
	std::copy(elementVector.begin(), elementVector.end(), back_inserter(temGreyLevels));
	//sort the vector and extract every element exactly once
	std::sort(temGreyLevels.begin(), temGreyLevels.end());
	auto it = std::unique(temGreyLevels.begin(), temGreyLevels.end());
	temGreyLevels.resize(std::distance(temGreyLevels.begin(), it));
	intVolFeatures.getFractionalVolume(inputMatrix, temGreyLevels);
	intVolFeatures.getGreyLevelFraction(inputMatrix);
	volAtIntFrac10 = intVolFeatures.getVolumeAtIntFraction(0.1);
	volAtIntFrac90 = intVolFeatures.getVolumeAtIntFraction(0.9);
	intAtVolFrac10 = intVolFeatures.getIntAtVolFraction(0.1, temGreyLevels);
	intAtVolFrac90 = intVolFeatures.getIntAtVolFraction(0.9, temGreyLevels);

	diffIntAtVolFrac = abs(intAtVolFrac90 - intAtVolFrac10);
	diffVolAtIntFrac = abs(volAtIntFrac90 - volAtIntFrac10);
	
}

template <class T, size_t R>
void IntensityVolumeFeatures<T, R>::writeCSVFileIntVol(IntensityVolumeFeatures<T,R> intVol, string outputFolder)
{
    string csvName = outputFolder + "_intensityVolFeat.csv";
    char * name = new char[csvName.size() + 1];
    std::copy(csvName.begin(), csvName.end(), name);
    name[csvName.size()] = '\0';

    ofstream intVolCSV;
    intVolCSV.open (name);
    vector<string> features;
    defineIntVolFeatures(features);

    vector<T> intVolData;
    extractIntVolData(intVolData, intVol);
    for(int i = 0; i< intVolData.size(); i++){
        intVolCSV <<"intensity volume"<<","<< features[i] <<",";
        intVolCSV << intVolData[i];
        intVolCSV << "\n";
    }
    intVolCSV.close();
}

template <class T, size_t R>
void IntensityVolumeFeatures<T, R>::writeOneFileIntVol(IntensityVolumeFeatures<T, R> intVol, ConfigFile config) {
	string csvName;
	if (config.csvOutput == 1) {
		csvName = config.outputFolder + ".csv";
	}
	else if (config.ontologyOutput == 1){
		csvName = config.outputFolder + "/feature_table.csv";
	}
	char * name = new char[csvName.size() + 1];
	std::copy(csvName.begin(), csvName.end(), name);
	name[csvName.size()] = '\0';

	ofstream intVolCSV;
	intVolCSV.open(name, std::ios_base::app);
	vector<string> features;
	

	vector<T> intVolData;
	extractIntVolData(intVolData, intVol);
	if (config.csvOutput == 1) {
		defineIntVolFeatures(features);
		for (int i = 0; i < intVolData.size(); i++) {
			intVolCSV << "intensity volume" << "," << features[i] << ",";
			intVolCSV << intVolData[i];
			intVolCSV << "\n";
		}
	}
	else if (config.ontologyOutput == 1) {
		defineIntVolFeaturesOntology(features);
		for (int i = 0; i < intVolData.size(); i++) {
			intVolCSV << config.patientID << "," << config.patientLabel << "," << features[i] << ",";
			intVolCSV << intVolData[i] << "," << config.featureParameterSpaceName << "," << config.calculationSpaceName;
			intVolCSV << "\n";
		}
	}
	intVolCSV.close();

}

template <class T, size_t R>
void IntensityVolumeFeatures<T, R>::defineIntVolFeatures(vector<string> &features){
    features.push_back("volume at int fraction 10");
    features.push_back("volume at int fraction 90");
    features.push_back("int at vol fraction 10");
    features.push_back("int at vol fraction 90");
	features.push_back("difference vol at int fraction");
    features.push_back("difference int at volume fraction");
   

}

template <class T, size_t R>
void IntensityVolumeFeatures<T, R>::defineIntVolFeaturesOntology(vector<string> &features) {
	features.push_back("Fivh.V10");
	features.push_back("Fivh.V90");
	features.push_back("Fivh.I10");
	features.push_back("Fivh.I90");
	features.push_back("Fivh.V10minusV90");
	features.push_back("Fivh.I10minusI90");
	
}

template <class T, size_t R>
void IntensityVolumeFeatures<T, R>::extractIntVolData(vector<T> &intVolData, IntensityVolumeFeatures<T, R> intVolFeatures){
    intVolData.push_back(intVolFeatures.volAtIntFrac10);
    intVolData.push_back(intVolFeatures.volAtIntFrac90);
    intVolData.push_back(intVolFeatures.intAtVolFrac10);
    intVolData.push_back(intVolFeatures.intAtVolFrac90);
	intVolData.push_back(intVolFeatures.diffVolAtIntFrac);
    intVolData.push_back(intVolFeatures.diffIntAtVolFrac);
    
}
#endif // INTENSITYVOLUMEFEATURES_H_INCLUDED
