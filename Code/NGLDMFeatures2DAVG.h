#ifndef NGLDMFEATURES2DAVG_H_INCLUDED
#define NGLDMFEATURES2DAVG_H_INCLUDED

 #include "NGLDMFeatures2DMRG.h"

/*! \file */
/*!
The class NGLDM2DAVG inherites from the class NGLDMFeatures. Here the 
matrices are calculated slice by slice. For every slice the feature values are calculated.\n
*/

template <class T,  size_t R>
class NGLDMFeatures2DAVG : public NGLDMFeatures2DMRG<T,R>{
    private:
		int dist;
		int coarseParam;

		vector<T> sumProbRows;
		vector<T> sumProbCols;
		vector<float> rowSums;
		vector<float> colSums;

        NGLDMFeatures2DMRG<T, R> ngldm;
        void extractNGLDMData2DAVG(vector<T> &ngldmData, NGLDMFeatures2DAVG<T, R> ngldmFeatures);
        boost::multi_array<float, 2> getMatrix(boost::multi_array<T, R> inputMatrix, boost::multi_array<T, R> ngldmNr, int depth);

    public:
        //double dependenceCountEnergy;
        void writeCSVFileNGLDM2DAVG(NGLDMFeatures2DAVG<T,R> ngldmFeat, string outputFolder);
		void writeOneFileNGLDM2DAVG(NGLDMFeatures2DAVG<T, R> ngldmFeat, ConfigFile config, int &parameterSpaceNr);
        void calculateAllNGLDMFeatures2DAVG(NGLDMFeatures2DAVG<T, R> &ngldmFeatures, Image<T, R> imageAttr, boost::multi_array<T, R> ngldmMatrix, ConfigFile config);

};

/*!
\brief getMatrix
@param[in] boost multi array inputMatrix: matrix filled with intensity values
@param[in] boost multi array ngldmNr: already filled NGLDM matrix, this matrix is 3D
@param[in] int depth: index of the actual slice
@param[out] boost multi array: filled 2D NGLD matrix

This function converts the 3D NGLDM matrix in the required 2D NGLD matrix for every slice
*/
template <class T, size_t R>
boost::multi_array<float, 2> NGLDMFeatures2DAVG<T, R>::getMatrix(boost::multi_array<T,R> inputMatrix, boost::multi_array<T, R> ngldmNr, int depth){
    typedef boost::multi_array<float, 2>  ngldmat;
    vector<int> actualIndex;
    T actualElement;
    int sizeGreyLevels = (this->diffGreyLevels).size();
	int actualGreyIndex;
    int ngldmnr;
    //define the NGLDMarices; col.size=9 because we have 8 neighbors
    ngldmat NGLDMatrix(boost::extents[sizeGreyLevels][9]);

    for(int row =0; row<ngldmNr.shape()[0]; row++){
		for(int col =0; col<ngldmNr.shape()[1]; col++){
			NGLDMatrix[row][col] = ngldmNr[row][col][depth];
              
        }
    }
    return NGLDMatrix;
}


template <class T, size_t R>
void NGLDMFeatures2DAVG<T, R>::calculateAllNGLDMFeatures2DAVG(NGLDMFeatures2DAVG<T,R> &ngldmFeatures, Image<T, R> imageAttr, boost::multi_array<T,R> ngldmMatrix, ConfigFile config){
	ngldmFeatures.getConfigValues(config);
	//get config values
	coarseParam = config.coarsenessParam;
	dist = config.distNGLDM;
	this->diffGreyLevels = imageAttr.diffGreyLevels;

    int totalDepth = imageAttr.imageMatrix.shape()[2];

	float meanRun;
	float meanGrey;
	float totalSum;

    T sumShortRunEmphasis = 0;
    T sumLongRunEmphasis = 0;
    T sumLowGreyEmph = 0;
    T sumHighGreyEmph = 0;
    T sumShortRunLow = 0;
    T sumShortRunHigh = 0;
    T sumLongRunLowEmph = 0;
    T sumLongRunHighEmph = 0;
    T sumGreyNonUniformity = 0;
    T sumGreyNonUniformityNorm = 0;
    T sumRunLengthNonUniformity = 0;
    T sumRunLengthNonUniformityNorm = 0;
    T sumRunPercentage = 0;
    T sumGreyLevelVar = 0;
    T sumRunLengthVar = 0;
    T sumRunEntropy = 0;
    T sumDependenceCountEnergy = 0;

    for(int depth = 0; depth < totalDepth; depth++){

        boost::multi_array<float,2> NGLDM=ngldmFeatures.getMatrix(imageAttr.imageMatrix, ngldmMatrix, depth);
        totalSum = ngldmFeatures.calculateTotalSum(NGLDM);

        boost::multi_array<float,2> probMatrix = ngldmFeatures.calculateProbMatrix(NGLDM, totalSum);

        meanGrey = ngldmFeatures.calculateMeanProbGrey(probMatrix);
        meanRun = ngldmFeatures.calculateMeanProbRun(probMatrix);

        rowSums = ngldmFeatures.calculateRowSums(NGLDM);
        colSums = ngldmFeatures.calculateColSums(NGLDM);

        ngldmFeatures.calculateShortRunEmphasis(rowSums, totalSum);
        sumShortRunEmphasis += this->shortRunEmphasis;
        ngldmFeatures.calculateLongRunEmphasis(rowSums, totalSum);
        sumLongRunEmphasis += this->longRunEmphasis;
        ngldmFeatures.calculateLowGreyEmph(colSums, totalSum);
        sumLowGreyEmph += this->lowGreyEmph;
        ngldmFeatures.calculateHighGreyEmph(colSums, totalSum);
        sumHighGreyEmph += this->highGreyEmph;
        ngldmFeatures.calculateShortRunLow(NGLDM, totalSum);
        sumShortRunLow += this->shortRunLow;
        ngldmFeatures.calculateShortRunHigh(NGLDM, totalSum);
        sumShortRunHigh += this->shortRunHigh;
        ngldmFeatures.calculateLongRunLowEmph(NGLDM, totalSum);
        sumLongRunLowEmph += this->longRunLowEmph;
        ngldmFeatures.calculateLongRunHighEmph(NGLDM, totalSum);
        sumLongRunHighEmph += this->longRunHighEmph;
        ngldmFeatures.calculateGreyNonUniformity(colSums, totalSum);
        sumGreyNonUniformity += this->greyNonUniformity;
        ngldmFeatures.calculateGreyNonUniformityNorm(colSums, totalSum);
        sumGreyNonUniformityNorm += this->greyNonUniformityNorm;
        ngldmFeatures.calculateRunLengthNonUniformityNorm(rowSums, totalSum);
        sumRunLengthNonUniformityNorm += this->runLengthNonUniformityNorm;
        ngldmFeatures.calculateRunLengthNonUniformity(rowSums, totalSum);
        sumRunLengthNonUniformity += this->runLengthNonUniformity;
        ngldmFeatures.calculateRunPercentage(imageAttr.imageMatrix, depth, totalSum, 1);
        sumRunPercentage += this->runPercentage;
        ngldmFeatures.calculateGreyLevelVar(probMatrix, meanGrey);
        sumGreyLevelVar += this->greyLevelVar;
        ngldmFeatures.calculateRunLengthVar(probMatrix, meanRun);
        sumRunLengthVar += this->runLengthVar;
        ngldmFeatures.calculateRunEntropy(probMatrix);
        sumRunEntropy += this->runEntropy;
        ngldmFeatures.calculateDependenceCountEnergy(probMatrix);
        sumDependenceCountEnergy += this->dependenceCountEnergy;
    }
    this->shortRunEmphasis = sumShortRunEmphasis/totalDepth;
    this->longRunEmphasis = sumLongRunEmphasis/totalDepth;
    this->lowGreyEmph = sumLowGreyEmph/totalDepth;
    this->highGreyEmph = sumHighGreyEmph/totalDepth;
    this->shortRunLow = sumShortRunLow/totalDepth;
    this->shortRunHigh = sumShortRunHigh/totalDepth;
    this->longRunLowEmph = sumLongRunLowEmph/totalDepth;
    this->longRunHighEmph = sumLongRunHighEmph/totalDepth;
    this->greyNonUniformity = sumGreyNonUniformity/totalDepth;
    this->greyNonUniformityNorm = sumGreyNonUniformityNorm/totalDepth;
    this->runLengthNonUniformity = sumRunLengthNonUniformity/totalDepth;
    this->runLengthNonUniformityNorm = sumRunLengthNonUniformityNorm/totalDepth;
    this->runPercentage = sumRunPercentage/totalDepth;
    this->greyLevelVar = sumGreyLevelVar/totalDepth;
    this->runLengthVar = sumRunLengthVar/totalDepth;
    this->runEntropy = sumRunEntropy/totalDepth;
    this->dependenceCountEnergy = sumDependenceCountEnergy/totalDepth;
}

template <class T, size_t R>
void NGLDMFeatures2DAVG<T, R>::extractNGLDMData2DAVG(vector<T> &ngldmData, NGLDMFeatures2DAVG<T, R> ngldmFeatures){

    ngldmData.push_back(ngldmFeatures.shortRunEmphasis);
    ngldmData.push_back(ngldmFeatures.longRunEmphasis);
    ngldmData.push_back(ngldmFeatures.lowGreyEmph);
    ngldmData.push_back(ngldmFeatures.highGreyEmph);
    ngldmData.push_back(ngldmFeatures.shortRunLow);
    ngldmData.push_back(ngldmFeatures.shortRunHigh);
    ngldmData.push_back(ngldmFeatures.longRunLowEmph);
    ngldmData.push_back(ngldmFeatures.longRunHighEmph);
    ngldmData.push_back(ngldmFeatures.greyNonUniformity);
    ngldmData.push_back(ngldmFeatures.greyNonUniformityNorm);
    ngldmData.push_back(ngldmFeatures.runLengthNonUniformity);   
    ngldmData.push_back(ngldmFeatures.runLengthNonUniformityNorm);
    ngldmData.push_back(ngldmFeatures.runPercentage);
    ngldmData.push_back(ngldmFeatures.greyLevelVar);
    ngldmData.push_back(ngldmFeatures.runLengthVar);
    ngldmData.push_back(ngldmFeatures.runEntropy);
    ngldmData.push_back(ngldmFeatures.dependenceCountEnergy);
}

template <class T, size_t R>
void NGLDMFeatures2DAVG<T, R>::writeCSVFileNGLDM2DAVG(NGLDMFeatures2DAVG<T,R> ngldmFeat, string outputFolder)
{
    string csvName = outputFolder + "_ngldmFeatures2Davg.csv";
    char * name = new char[csvName.size() + 1];
    std::copy(csvName.begin(), csvName.end(), name);
    name[csvName.size()] = '\0';

    ofstream ngldmCSV;
    ngldmCSV.open (name);
    vector<string> features;
    ngldm.defineNGLDMFeatures(features);

    vector<T> ngldmData;
    extractNGLDMData2DAVG(ngldmData, ngldmFeat);
    for(int i = 0; i< ngldmData.size(); i++){
        ngldmCSV << "ngldmFeatures2Davg"<< ","<<features[i] <<",";
        ngldmCSV << ngldmData[i];
        ngldmCSV << "\n";
    }
    ngldmCSV.close();
}

template <class T, size_t R>
void NGLDMFeatures2DAVG<T, R>::writeOneFileNGLDM2DAVG(NGLDMFeatures2DAVG<T, R> ngldmFeat, ConfigFile config, int &parameterSpaceNr) {
	string csvName;
	if (config.csvOutput == 1) {
		csvName = config.outputFolder + ".csv";
	}
	else if (config.ontologyOutput == 1) {
		csvName = config.outputFolder + "/feature_table.csv";
	}
	char * name = new char[csvName.size() + 1];
	std::copy(csvName.begin(), csvName.end(), name);
	name[csvName.size()] = '\0';

	ofstream ngldmCSV;
	ngldmCSV.open(name, std::ios_base::app);
	vector<string> features;
	

	vector<T> ngldmData;
	extractNGLDMData2DAVG(ngldmData, ngldmFeat);
	
	if (config.csvOutput == 1) {
		ngldm.defineNGLDMFeatures(features);
		for (int i = 0; i < ngldmData.size(); i++) {
			ngldmCSV << "ngldmFeatures2Davg" << "," << features[i] << ",";
			ngldmCSV << ngldmData[i];
			ngldmCSV << "\n";
		}
	}
	else if (config.ontologyOutput == 1) {
		features.clear();
		ngldm.defineNGLDMFeaturesOntology(features);
		string featParamSpaceTable = config.outputFolder + "/FeatureParameterSpace_table.csv";
		char * featParamSpaceTableName = new char[featParamSpaceTable.size() + 1];
		std::copy(featParamSpaceTable.begin(), featParamSpaceTable.end(), featParamSpaceTableName);
		featParamSpaceTableName[featParamSpaceTable.size()] = '\0';

		ofstream featSpaceTable;
		featSpaceTable.open(featParamSpaceTableName, std::ios_base::app);
		parameterSpaceNr += 1;
		string parameterSpaceName = "FeatureParameterSpace_" + std::to_string(parameterSpaceNr);
		featSpaceTable << parameterSpaceName << "," << "2Davg" << "," << config.imageSpaceName << "," << config.interpolationMethod << "\n";
		featSpaceTable.close();

		for (int i = 0; i < ngldmData.size(); i++) {
			ngldmCSV << config.patientID << "," << config.patientLabel << "," << features[i] << ",";
			ngldmCSV << ngldmData[i] << "," << parameterSpaceName << "," << config.calculationSpaceName;
			ngldmCSV << "\n";
		}

	}
	ngldmCSV.close();

}

#endif // NGLDMFEATURES2DAVG_H_INCLUDED
