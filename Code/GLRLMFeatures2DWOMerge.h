#ifndef GLRLMFEATURES2DWOMERGE_H_INCLUDED
#define GLRLMFEATURES2DWOMERGE_H_INCLUDED


#include "GLRLMFeatures2DFullMerge.h"
/*! \file */
/*!
The class GLCMFeatures2DWOMerge inherits from the matrix GLCMFeatures. \n
It does not merge the matrices before feature calculation. \n
For every slice a GLCMatrix is calculated and from every of this matrices all features are extracted. \n
Then the average value of all features is calculated.
*/
template <class T,  size_t R=3>
class GLRLMFeatures2DWOMerge : GLRLMFeatures<T,R>  {

    private:

        GLRLMFeatures<T,R> glrlm;
        GLRLMFeatures2DFullMerge<T,R> glrlm2DFullMerge;

        double totalSum;

        typedef boost::multi_array<double,2> glrlmMat;

        int directionX;
        int directionY;

        int maxRunLength;

        boost::multi_array<double,2> createGLRLMatrixWOMerge(boost::multi_array<T, R> inputMatrix, int depth, int ang);
        void extractGLRLMDataWOMerge(vector<T> &glrlmData, GLRLMFeatures2DWOMerge<T, R> glrlmFeatures);
        void fill2DMatrices2DWOMerge(boost::multi_array<T, R> inputMatrix, boost::multi_array<double,2> &glrlMatrix, int depth, int ang);

    public:
		GLRLMFeatures2DWOMerge() {
		}
		~GLRLMFeatures2DWOMerge() {
		}
        void calculateAllGLRLMFeatures2DWOMerge(GLRLMFeatures2DWOMerge<T,R> &glrlmFeatures, boost::multi_array<T, R> inputMatrix, vector<T> diffFGrey, ConfigFile config);
        void writeCSVFileGLRLM2DWOMerge(GLRLMFeatures2DWOMerge<T,R> glrlmFeat, string outputFolder);
		void writeOneFileGLRLM2DWOMerge(GLRLMFeatures2DWOMerge<T, R> glrlmFeat, string outputFolder);

};

/*!
In the method createGLRLMatrixW=Merge the GLRLM-matrix for given slice is calculated \n
@param[in]: boost::multi_array<T, 3> inputMatrix: original matrix of the VOI
@param[in] : int depth: number of the actual slice
@param[in] : int angle: angle
@param[out]: GLCM-matrix
*/
template <class T, size_t R>
boost::multi_array<double,2> GLRLMFeatures2DWOMerge<T, R>::createGLRLMatrixWOMerge(boost::multi_array<T,R> inputMatrix, int depth, int ang){

    int sizeMatrix = this->diffGreyLevels.size();

    glrlmMat GLRLMatrix(boost::extents[sizeMatrix][this->maxRunLength]);
    fill2DMatrices2DWOMerge(inputMatrix, GLRLMatrix, depth, ang);

    return GLRLMatrix;

}


/*!
In the method fill2DMatrices2DWOMerge the matrix is filled for the given image slice and angle
@param[in] inputMatrix: the original matrix of the VOI
@param[in]: the GLCMatrix that will be filled with the corresponding values
@param[in]: number of actual slice
@param[in]: direction that determines how the matrix will be filled

The function works analog to the function in GLRLMFeatures2DFullMerge
*/
template <class T, size_t R>
void GLRLMFeatures2DWOMerge<T, R>::fill2DMatrices2DWOMerge(boost::multi_array<T, R> inputMatrix, boost::multi_array<double,2> &glrlMatrix, int depth, int ang){

    double actGreyLevel = 0;
    double actElement = 0;
    int runLength=0;
    int maxRowNr = inputMatrix.shape()[0];
    int maxColNr = inputMatrix.shape()[1];
    glrlm.getXYDirections(directionX, directionY, ang);
    //have a look at the image-matrix slide by slide (2D)
    //look for every grey level separately in every image slide
    for(int actGreyIndex=0; actGreyIndex < this->diffGreyLevels.size(); actGreyIndex++){
        //get the grey level we are interested at the moment
        actGreyLevel = this->diffGreyLevels[actGreyIndex];
        //now look at the image matrix slide by slide
         for(int row = 0; row<maxRowNr;row++){
            for(int column = 0; column<maxColNr; column++){
//                  //at the beginning the run length =0
                runLength =0;
                //get the actual matrix element
                actElement = inputMatrix[maxRowNr-row-1][column][depth];
                //if the actual matrix element is the same as the actual gre level
                if(actElement==actGreyLevel){
                    //set the run length to 1
                    runLength=1;
                    //to avoid to take an element more than once, set the element to NAN
                    inputMatrix[maxRowNr-row-1][column][depth] = NAN;
////                //now look at the matrix element in the actual direction (depends on the
                    //angle we are interested at the moment
                    int colValue = column+directionX;
                    int rowValue = maxRowNr-1-(row+directionY);
                    //now have a look at the following elements in the desired direction
                    //stop as soon as we look at an element diifferent from our actual element
                    while(colValue<maxColNr && rowValue>-1 &&colValue>-1&& inputMatrix[rowValue][colValue][depth]==actGreyLevel){
                        //for every element we find, count the runLength
                        runLength+=1;
                        inputMatrix[rowValue][colValue][depth]=NAN;
                        //go further in the desired direction
                        colValue += 1*directionX;
                        rowValue -= 1*directionY;
                    }
                }
                //as soon as we cannot find an element in the desired direction, count one up in the desired
                //position of the glrl-matrix

                if(runLength > 0 && runLength < glrlMatrix.shape()[1] + 1){
                    glrlMatrix[actGreyIndex][runLength-1] += 1;
                }

            }
        }
    }

}


template <class T, size_t R>
void GLRLMFeatures2DWOMerge<T, R>::calculateAllGLRLMFeatures2DWOMerge(GLRLMFeatures2DWOMerge<T,R> &glrlmFeatures, boost::multi_array<T, R> inputMatrix, vector<T> diffGrey, ConfigFile config){
    this->diffGreyLevels = diffGrey;
	glrlmFeatures.getConfigValues(config);
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

    vector<double> rowSums;
    vector<double> colSums;



    double meanGrey;
    double meanRun;

    int totalDepth = inputMatrix.shape()[2];

    maxRunLength = glrlm.getMaxRunLength(inputMatrix);

    int ang;
    for(int depth = 0; depth < totalDepth; depth++){

        for(int i = 0; i < 4; i++){
            ang = 180-i*45;
            boost::multi_array<double,2> glrlMatrix = createGLRLMatrixWOMerge(inputMatrix, depth, ang);
            totalSum = glrlmFeatures.calculateTotalSum(glrlMatrix);
            rowSums = glrlmFeatures.calculateRowSums(glrlMatrix);
            colSums = glrlmFeatures.calculateColSums(glrlMatrix);

            boost::multi_array<double,2> probMatrix = glrlmFeatures.calculateProbMatrix(glrlMatrix, totalSum);
            meanGrey = glrlmFeatures.calculateMeanProbGrey(probMatrix);
            meanRun = glrlmFeatures.calculateMeanProbRun(probMatrix);


            glrlmFeatures.calculateShortRunEmphasis(rowSums, totalSum);
            sumShortRunEmphasis += this->shortRunEmphasis;
            glrlmFeatures.calculateLongRunEmphasis(rowSums, totalSum);
            sumLongRunEmphasis += this->longRunEmphasis;
            glrlmFeatures.calculateLowGreyEmph(colSums, totalSum);
            sumLowGreyEmph += this->lowGreyEmph;
            glrlmFeatures.calculateHighGreyEmph(colSums, totalSum);
            sumHighGreyEmph += this->highGreyEmph;
            glrlmFeatures.calculateShortRunLow(glrlMatrix, totalSum);
            sumShortRunLow += this->shortRunLow;
            glrlmFeatures.calculateShortRunHigh(glrlMatrix, totalSum);
            sumShortRunHigh += this->shortRunHigh;
            glrlmFeatures.calculateLongRunLowEmph(glrlMatrix, totalSum);
            sumLongRunLowEmph += this->longRunLowEmph;
            glrlmFeatures.calculateLongRunHighEmph(glrlMatrix, totalSum);
            sumLongRunHighEmph += this->longRunHighEmph;
            glrlmFeatures.calculateGreyNonUniformity(colSums, totalSum);
            sumGreyNonUniformity += this->greyNonUniformity;
            glrlmFeatures.calculateGreyNonUniformityNorm(colSums, totalSum);
            sumGreyNonUniformityNorm += this->greyNonUniformityNorm;
            glrlmFeatures.calculateRunLengthNonUniformity(rowSums, totalSum);
            sumRunLengthNonUniformity += this->runLengthNonUniformity;
            glrlmFeatures.calculateRunLengthNonUniformityNorm(rowSums, totalSum);
            sumRunLengthNonUniformityNorm += this->runLengthNonUniformityNorm;
            glrlmFeatures.calculateRunPercentage(inputMatrix, depth, totalSum, 1);
            sumRunPercentage += this->runPercentage;

            glrlmFeatures.calculateGreyLevelVar(probMatrix, meanGrey);
            sumGreyLevelVar += this->greyLevelVar;
            glrlmFeatures.calculateRunLengthVar(probMatrix, meanRun);
            sumRunLengthVar += this->runLengthVar;
            glrlmFeatures.calculateRunEntropy(probMatrix);
            sumRunEntropy += this->runEntropy;

            }
        }

        this->shortRunEmphasis = sumShortRunEmphasis/(totalDepth*4);
        this->longRunEmphasis = sumLongRunEmphasis/(totalDepth*4);
        this->lowGreyEmph = sumLowGreyEmph/(totalDepth*4);
        this->highGreyEmph = sumHighGreyEmph/(totalDepth*4);
        this->shortRunLow = sumShortRunLow/(totalDepth*4);
        this->shortRunHigh = sumShortRunHigh/(totalDepth*4);
        this->longRunLowEmph = sumLongRunLowEmph/(totalDepth*4);
        this->longRunHighEmph = sumLongRunHighEmph/(totalDepth*4);
        this->greyNonUniformity = sumGreyNonUniformity/(totalDepth*4);
        this->greyNonUniformityNorm = sumGreyNonUniformityNorm/(totalDepth*4);
        this->runLengthNonUniformity = sumRunLengthNonUniformity/(totalDepth*4);
        this->runLengthNonUniformityNorm = sumRunLengthNonUniformityNorm/(totalDepth*4);
        this->runPercentage = sumRunPercentage/(totalDepth*4);

        this->greyLevelVar = sumGreyLevelVar/(totalDepth*4);
        this->runLengthVar = sumRunLengthVar/(totalDepth*4);
        this->runEntropy = sumRunEntropy/(totalDepth*4);
}



template <class T, size_t R>
void GLRLMFeatures2DWOMerge<T, R>::writeCSVFileGLRLM2DWOMerge(GLRLMFeatures2DWOMerge<T, R> glrlmFeat, string outputFolder)
{
    string csvName = outputFolder + "/GLRLMFeatures2Davg.csv";
    char * name = new char[csvName.size() + 1];
    std::copy(csvName.begin(), csvName.end(), name);
    name[csvName.size()] = '\0';

    ofstream glrlmCSV;
    glrlmCSV.open (name);
    vector<string> features;
    glrlm.defineGLRLMFeatures(features);

    vector<T> glrlmData;
    extractGLRLMDataWOMerge(glrlmData, glrlmFeat);
    for(int i = 0; i< glrlmData.size(); i++){
        glrlmCSV <<"GLRLMFeatures2Davg"<<","<< features[i] <<",";
        glrlmCSV << glrlmData[i];
        glrlmCSV << "\n";
    }
    glrlmCSV.close();
}

template <class T, size_t R>
void GLRLMFeatures2DWOMerge<T, R>::writeOneFileGLRLM2DWOMerge(GLRLMFeatures2DWOMerge<T, R> glrlmFeat, string outputFolder) {
	string csvName = outputFolder + "/radiomicsFeatures.csv";
	char * name = new char[csvName.size() + 1];
	std::copy(csvName.begin(), csvName.end(), name);
	name[csvName.size()] = '\0';

	ofstream glrlmCSV;
	glrlmCSV.open(name, std::ios_base::app);
	vector<string> features;
	glrlm.defineGLRLMFeatures(features);

	vector<T> glrlmData;
	extractGLRLMDataWOMerge(glrlmData, glrlmFeat);
	for (int i = 0; i< glrlmData.size(); i++) {
		glrlmCSV << "GLRLMFeatures2Davg" << "," << features[i] << ",";
		glrlmCSV << glrlmData[i];
		glrlmCSV << "\n";
	}
	glrlmCSV.close();
}


template <class T, size_t R>
void GLRLMFeatures2DWOMerge<T, R>::extractGLRLMDataWOMerge(vector<T> &glrlmData, GLRLMFeatures2DWOMerge<T, R> glrlmFeatures){

    glrlmData.push_back(glrlmFeatures.shortRunEmphasis);
    glrlmData.push_back(glrlmFeatures.longRunEmphasis);
    glrlmData.push_back(glrlmFeatures.lowGreyEmph);
    glrlmData.push_back(glrlmFeatures.highGreyEmph);
    glrlmData.push_back(glrlmFeatures.shortRunLow);
    glrlmData.push_back(glrlmFeatures.shortRunHigh);
    glrlmData.push_back(glrlmFeatures.longRunLowEmph);
    glrlmData.push_back(glrlmFeatures.longRunHighEmph);
    glrlmData.push_back(glrlmFeatures.greyNonUniformity);
    glrlmData.push_back(glrlmFeatures.greyNonUniformityNorm);
    glrlmData.push_back(glrlmFeatures.runLengthNonUniformity);   //check if I have to give the matrix
    glrlmData.push_back(glrlmFeatures.runLengthNonUniformityNorm);
    glrlmData.push_back(glrlmFeatures.runPercentage);
    glrlmData.push_back(glrlmFeatures.greyLevelVar);
    glrlmData.push_back(glrlmFeatures.runLengthVar);
    glrlmData.push_back(glrlmFeatures.runEntropy);

}





#endif // GLRLMFEATURES2DWOMERGE_H_INCLUDED
