#ifndef GLDZMFEATURES2D_H_INCLUDED
#define GLDZMFEATURES2D_H_INCLUDED
#include <algorithm>
#include "GLSZMFeatures2D.h"

/*! \file */
/*!
The class GLDZM is the class of the Grey Level Distance Zone Matrices. \n
It combines the grey level size zone matrices with a distance map. Voxels are considered as connected,
when they have the same grey value. \n
The distance to the edge is also defined according to 4-connectedness. The distance of a voxel to the
outer border is defined as the number of edges that have to be crossed to reach the edge of the VOI. \n
*/

template <class T,  size_t R=3>
class GLDZMFeatures2D : public GLSZMFeatures2DMRG<T,R>{
    private:
		
		vector<T> diagonalProbabilities;
		vector<T> crossProbabilities;
		vector<T> sumProbRows;
		vector<T> sumProbCols;

		int totalNrZones;
		vector<float> rowSums;
		vector<float> colSums;
		
		GLSZMFeatures2DMRG<T, R> GLSZM2D;
        void extractGLDZMData(vector<T> &gldzmData, GLDZMFeatures2D<T, R> gldzmFeatures);
        boost::multi_array<float, 2> fillMatrix(boost::multi_array<T,R> inputMatrix, boost::multi_array<T, R> distanceMap, boost::multi_array<float, 2>  &gldzmat);
        boost::multi_array<float, 2> getMatrix( boost::multi_array<T, R> inputMatrix, boost::multi_array<T, R> distanceMap);

     public:
		void defineGLDZMFeatures(vector<string> &features);
		void defineGLDZMFeaturesOntology(vector<string> &features);
        int getMinimalDistance(boost::multi_array<T,R> distanceMap, vector<vector<int> > matrixIndices);
        void writeCSVFileGLDZM(GLDZMFeatures2D<T,R> gldzmFeat, string outputFolder);
		void writeOneFileGLDZM(GLDZMFeatures2D<T, R> gldzmFeat, ConfigFile config, int &parameterSpaceNr);
		void calculateAllGLDZMFeatures2D(GLDZMFeatures2D<T,R> &gldzmFeat, boost::multi_array<T, R> distanceMap, Image<T, R> imageAttr, ConfigFile config);
};





/*!
In the method fillMatrix the GLDZM matrix is filled, taking the original matrix of the VOI as input. \n
The GLDZM matrix is given as reference and filled in the function
@param[in]: boost::multi_array<T, 3> inputMatrix: original matrix of the VOI
@param[in]: boost::multi_array<T, 3> GLDZM: GLDZM matrix 
*/
template <class T, size_t R>
boost::multi_array<float, 2> GLDZMFeatures2D<T, R>::fillMatrix(boost::multi_array<T,R> inputMatrix, boost::multi_array<T, R> distanceMap, boost::multi_array<float, 2>  &gldzmat){
	//store the matrix indices of a neighborhood in a vector
    vector<vector<int> > matrixIndices;
	//vector with indices of actual position
    vector<int> actualIndex;
    T actualElement;
    int minDistance;
    totalNrZones=0;
	//get zones for every grey level present in VOI
	int actualGreyIndex;
	for (int depth = 0; depth < inputMatrix.shape()[2]; depth++) {
		for (int row = 0; row < inputMatrix.shape()[0]; row++) {
			for (int col = 0; col < inputMatrix.shape()[1]; col++) {
				actualElement = inputMatrix[row][col][depth];
				if (!isnan(actualElement)) {
					actualGreyIndex = GLSZM2D.findIndex(this->diffGreyLevels, boost::size(this->diffGreyLevels), actualElement);
					inputMatrix[row][col][depth] = NAN;
					actualIndex.push_back(row);
					actualIndex.push_back(col);
					actualIndex.push_back(depth);
					matrixIndices.push_back(actualIndex);
					actualIndex.clear();
					GLSZM2D.getNeighbors(inputMatrix, actualElement, matrixIndices);
				}
				if (matrixIndices.size() > 0) {
					minDistance = getMinimalDistance(distanceMap, matrixIndices);
					matrixIndices.clear();
					if (minDistance > 0) {
						gldzmat[actualGreyIndex][minDistance - 1] += 1;
					}
				}
			}
		}

	}
    return gldzmat;
}

/*!
In the method getMatrix the GLDZM matrix is generated and filled using the function fillMatrix. 
The function is mainly used get the size of the GLDZM matrix. \n
@param[in]: boost::multi_array<T, 3> inputMatrix: original matrix of the VOI
@param[out]: boost::multi_array<T, 3> GLDZM: GLDZM matrix
*/
template <class T, size_t R>
boost::multi_array<float, 2> GLDZMFeatures2D<T,R>::getMatrix( boost::multi_array<T, R> inputMatrix, boost::multi_array<T, R> distanceMap){
    typedef boost::multi_array<float, 2>  gldzmat;
	//all grey levels present in VOI
    int sizeGreyLevels = (this->diffGreyLevels).size();
    //define the glcMatrices
	//the size is the minimum of the half of the side lengths + 1, because the minimum of the half length has to be included

	int nrCols = std::min(std::ceil(double(inputMatrix.shape()[0]) / 2), std::ceil(double(inputMatrix.shape()[1]) / 2)) +1;
    gldzmat GLDZMatrix(boost::extents[sizeGreyLevels][nrCols]);
	fillMatrix(inputMatrix, distanceMap, GLDZMatrix);
    return GLDZMatrix;
}

/*!
In the method getMinimalDistance gets the minimal distance of a zone to the VOI edge. \n
@param[in]: boost::multi_array<T, 3> distance map
@param[in]: vector<vector<int>> matrixIndices: vector with all matrix indices of the actual zone
*/
template <class T, size_t R>
int GLDZMFeatures2D<T, R>::getMinimalDistance(boost::multi_array<T,R> distanceMap, vector<vector<int> > matrixIndices){
    vector<int> distances;
    int actualX;
    int actualY;
    int actualZ;
    int actualDistance;
    int minDistance;
    vector<int> actualIndex;
	//check for every element of the vector matrix indices the distance
    while(matrixIndices.size()>0){
        actualIndex = matrixIndices.back();
        matrixIndices.pop_back();
        actualX = actualIndex[0];
        actualY = actualIndex[1];
        actualZ = actualIndex[2];
        actualDistance = distanceMap[actualX][actualY][actualZ];
		if (actualDistance > 0) {
			distances.push_back(actualDistance);
		}
		else {
			std::cout << "dist0" << std::endl;
		}

    }
	if (distances.size() > 0) {
		//get minimum of all distances of the actual neighborhood
		minDistance = *min_element(distances.begin(), distances.end());
	}
	
	return minDistance;
}

template <class T, size_t R>
void GLDZMFeatures2D<T, R>::calculateAllGLDZMFeatures2D(GLDZMFeatures2D<T,R> &gldzmFeatures, boost::multi_array<T, R> distanceMap, Image<T,R> imageAttr, ConfigFile config){
    //store all grey levels of actual VOI in vector
	this->diffGreyLevels = imageAttr.diffGreyLevels;
	//read in the config values for the extended emphasis values
	gldzmFeatures.getConfigValues(config);

    boost::multi_array<float,2> GLDZM=gldzmFeatures.getMatrix(imageAttr.imageMatrix, distanceMap);

	float totalSum = gldzmFeatures.calculateTotalSum(GLDZM);
    
	rowSums=gldzmFeatures.calculateRowSums(GLDZM);
    colSums = gldzmFeatures.calculateColSums(GLDZM);

    gldzmFeatures.calculateShortRunEmphasis(rowSums, totalSum);
    gldzmFeatures.calculateLongRunEmphasis(rowSums, totalSum);
    gldzmFeatures.calculateLowGreyEmph(colSums, totalSum);
    gldzmFeatures.calculateHighGreyEmph(colSums, totalSum);
    gldzmFeatures.calculateShortRunLow(GLDZM, totalSum);
    gldzmFeatures.calculateShortRunHigh(GLDZM, totalSum);
    gldzmFeatures.calculateLongRunLowEmph(GLDZM, totalSum);
    gldzmFeatures.calculateLongRunHighEmph(GLDZM, totalSum);
    gldzmFeatures.calculateGreyNonUniformity(colSums, totalSum);
    gldzmFeatures.calculateGreyNonUniformityNorm(colSums, totalSum);
    gldzmFeatures.calculateRunLengthNonUniformityNorm(rowSums, totalSum);
    gldzmFeatures.calculateRunLengthNonUniformity(rowSums, totalSum);
    gldzmFeatures.calculateRunPercentage3D(imageAttr.vectorOfMatrixElements, totalSum, 4);
    boost::multi_array<float,2> probMatrix = gldzmFeatures.calculateProbMatrix(GLDZM, totalSum);
	float meanGrey = gldzmFeatures.calculateMeanProbGrey(probMatrix);

    gldzmFeatures.calculateGreyLevelVar(probMatrix, meanGrey);

	float meanRun = gldzmFeatures.calculateMeanProbRun(probMatrix);
    gldzmFeatures.calculateRunLengthVar(probMatrix, meanRun);
    gldzmFeatures.calculateRunEntropy(probMatrix);
}

template <class T, size_t R>
void GLDZMFeatures2D<T, R>::extractGLDZMData(vector<T> &gldzmData, GLDZMFeatures2D<T, R> gldzmFeatures){

    gldzmData.push_back(gldzmFeatures.shortRunEmphasis);
    gldzmData.push_back(gldzmFeatures.longRunEmphasis);
    gldzmData.push_back(gldzmFeatures.lowGreyEmph);
    gldzmData.push_back(gldzmFeatures.highGreyEmph);
    gldzmData.push_back(gldzmFeatures.shortRunLow);
    gldzmData.push_back(gldzmFeatures.shortRunHigh);
    gldzmData.push_back(gldzmFeatures.longRunLowEmph);
    gldzmData.push_back(gldzmFeatures.longRunHighEmph);
    gldzmData.push_back(gldzmFeatures.greyNonUniformity);
    gldzmData.push_back(gldzmFeatures.greyNonUniformityNorm);
    gldzmData.push_back(gldzmFeatures.runLengthNonUniformity);   //check if I have to give the matrix
    gldzmData.push_back(gldzmFeatures.runLengthNonUniformityNorm);
    gldzmData.push_back(gldzmFeatures.runPercentage);
    gldzmData.push_back(gldzmFeatures.greyLevelVar);
    gldzmData.push_back(gldzmFeatures.runLengthVar);
    gldzmData.push_back(gldzmFeatures.runEntropy);

}

template <class T, size_t R>
void GLDZMFeatures2D<T, R>::writeCSVFileGLDZM(GLDZMFeatures2D<T,R> gldzmFeat, string outputFolder)
{
    string csvName = outputFolder + "_gldzmFeatures2Dmrg.csv";
    char * name = new char[csvName.size() + 1];
    std::copy(csvName.begin(), csvName.end(), name);
    name[csvName.size()] = '\0';

    ofstream gldzmCSV;
    gldzmCSV.open (name);
    vector<string> features;
    defineGLDZMFeatures(features);

    vector<T> gldzmData;
    extractGLDZMData(gldzmData, gldzmFeat);
    for(int i = 0; i< gldzmData.size(); i++){
        gldzmCSV << "gldzmFeatures2Dmrg"<<","<<features[i] <<",";
        gldzmCSV << gldzmData[i];
        gldzmCSV << "\n";
    }
    gldzmCSV.close();
}


template <class T, size_t R>
void GLDZMFeatures2D<T, R>::writeOneFileGLDZM(GLDZMFeatures2D<T, R> gldzmFeat, ConfigFile config, int &parameterSpaceNr) {
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

	ofstream gldzmCSV;
	gldzmCSV.open(name, std::ios_base::app);
	vector<string> features;
	

	vector<T> gldzmData;
	extractGLDZMData(gldzmData, gldzmFeat);
	
	if (config.csvOutput == 1) {
		defineGLDZMFeatures(features);
		for (int i = 0; i < gldzmData.size(); i++) {
			gldzmCSV << "gldzmFeatures2Dmrg" << "," << features[i] << ",";
			gldzmCSV << gldzmData[i];
			gldzmCSV << "\n";
		}
	}
	else if (config.ontologyOutput == 1) {
		features.clear();
		defineGLDZMFeaturesOntology(features);
		string featParamSpaceTable = config.outputFolder + "/FeatureParameterSpace_table.csv";
		char * featParamSpaceTableName = new char[featParamSpaceTable.size() + 1];
		std::copy(featParamSpaceTable.begin(), featParamSpaceTable.end(), featParamSpaceTableName);
		featParamSpaceTableName[featParamSpaceTable.size()] = '\0';

		ofstream featSpaceTable;
		featSpaceTable.open(featParamSpaceTableName, std::ios_base::app);
		parameterSpaceNr += 1;
		string parameterSpaceName = "FeatureParameterSpace_" + std::to_string(parameterSpaceNr);
		featSpaceTable << parameterSpaceName << "," << "2Dmrg" << "," << config.imageSpaceName << "," << config.interpolationMethod << "\n";
		featSpaceTable.close();

		for (int i = 0; i < gldzmData.size(); i++) {
			gldzmCSV << config.patientID << "," << config.patientLabel << "," << features[i] << ",";
			gldzmCSV << gldzmData[i] << "," << parameterSpaceName << "," << config.calculationSpaceName;
			gldzmCSV << "\n";
		}

	}
	gldzmCSV.close();
}

template <class T, size_t R>
void GLDZMFeatures2D<T, R>::defineGLDZMFeatures(vector<string> &features){
    features.push_back("small distance emphasis GLDZM");
    features.push_back("Large distance emphasis GLDZM");
    features.push_back("Low grey level zone emphasis GLDZM");
    features.push_back("High grey level zone emphasis GLDZM");
    features.push_back("Small distance low grey level emphasis GLDZM");
    features.push_back("Small distance high grey level emphasis GLDZM");
    features.push_back("Large distance low grey level emphasis GLDZM");
    features.push_back("Large distance high grey level emphasis GLDZM");
    features.push_back("Grey level non uniformity GLDZM");
    features.push_back("Grey level non uniformity normalized GLDZM");
    features.push_back("Zone distance non uniformity GLDZM");
    features.push_back("Zone distance non uniformity normalized GLDZM");
    features.push_back("Zone percentage GLDZM");
    features.push_back("Grey level variance GLDZM");
    features.push_back("Zone distance variance GLDZM");
    features.push_back("Zone distance entropy GLDZM");

}

template <class T, size_t R>
void GLDZMFeatures2D<T, R>::defineGLDZMFeaturesOntology(vector<string> &features) {
	features.push_back("Fdzm.sde");
	features.push_back("Fdzm.lde");
	features.push_back("Fdzm.lgze");
	features.push_back("Fdzm.hgze");
	features.push_back("Fdzm.sdlge");
	features.push_back("Fdzm.sdhge");
	features.push_back("Fdzm.ldlge");
	features.push_back("Fdzm.ldhge");
	features.push_back("Fdzm.glnu");
	features.push_back("Fdzm.glnu.norm");
	features.push_back("Fdzm.zdnu");
	features.push_back("Fdzm.zdnu.norm");
	features.push_back("Fdzm.z.perc");
	features.push_back("Fdzm.gl.var");
	features.push_back("Fdzm.zd.var");
	features.push_back("Fdzm.zd.entr");

}
#endif // GLDZMFEATURES2D_H_INCLUDED
