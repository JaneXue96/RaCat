#include "readInFeatureSelection.h"
#include <iostream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

#include "string"

using namespace std;


/*! @page featureSelection Feature calculation and setting up the featureSelection.ini file
In the file featureSelection.ini, the user can set which features should be calculated. \n
The feature selection file is splitted in several paragraphs: For every feature group one. Setting the value in one paragraph to 0 will exclude this feature group from the calculation step. \n
E.g. if in the section [StatisticalFeatures] the value `CalculateStatFeat = 0`, the statistical features are not calculated. \n
The following feature groups are available. For exact explanations of the implementation, check the class documentation. 
For the feature definitions, check [Zwanenburg] (https://arxiv.org/abs/1612.07003) \n
Features that do not require interpolation: \n
\arg Morphological features \n
Features describing the tumor shape like volume, surface etc.\n
\n
\arg Local intensity features \n
Features describing local or global intensity values like SUVpeak.\n
\n
\arg  Statistical features
First order features like the mean, maximum or minimum intensity value of the VOI. \n
\n 
Textural features requiring interpolation. All these feature capture heterogeneity information about the VOI.\n
\n
\arg Grey Level Co-occurence based features \n
For the Grey Level Co-occurence features, a matrix is created that captures information about the neighboring voxels in the VOI.\n
The number of times a certain pair of neighboring voxels occur in the VOI is stored in this matrix. In this way,
the matrix measures if there are a lot of neighboring voxels with high intensity differences and captures information 
about the heterogeneity/homogeneity of the VOI.\n
For the grey level co-occurence features, five different ways of computing exist.\n
\n
\arg Grey Level Run length based features \n
In order to calculate the GLR based features, a matrix is calculated that contains the information how often a certain grey level
occurs in a row in the VOI. The run length is captured for different angles. \n
In this way, also the GLR features capture heterogeneity information. \n
Also for this feature group, five different ways of creating the matrix exist. All five are implemented here. \n
\n
\arg Grey Level Size Zone based features \n
The GLSZ features capture information about how many connected voxels with the same intensity values can be found in the VOI. \n
For this feature group, three ways of creating the matrix exist. \n
\n
\arg Grey Level Distance Zone based features \n
The GLDZ based features also contain information about how many connected voxels with the same intensity values can be found in the VOI.\n
On top of that, it also stores the information how far away the minimum distance of this connected zones is from the border of the VOI. \n
Also for this feature group, three different ways of building the matrix exist.\n
\n
\arg Neighborhood Grey Tone Difference based features \n
The NGTD contains the sum of grey level differences of voxels with the same grey level and the average grey level of neighbouring voxels. \n
The user can define the size of the neighborhood in the config.ini file. The default value is 1. \n
\n

\arg Neighborhood Grey Level Dependence based features \n
For the NGLD matrices, for every voxel of the VOI a neighbourhood is defined as the voxels located within a distance d 
around this voxel. If the intensity differences of this voxel and the neighboring voxels are smaller than a certain value (coarseness parameter),
the voxels are regarded as independent. The NGLD matrices contain this information. \n
The user can set the size of the neighborhood, as well as the coarseness parameter in the config.ini file.\n
This matrices contain information about the coarseness of the VOI.


*/



/*!
In the function CalculateRelFeatures, all features that do not require interpolation are calculated. \n
If one feature group should not be calculated, this group is skipped from the calculation \n
The feature values are stored in the outputfile set by the user.
*/
void CalculateRelFeatures(Image<float, 3> imageAttr, ConfigFile config)
{
	int a =1;
	int morph;
	int localInt;
	int statFeat;
	int intVolFeatures;
	boost::property_tree::ptree pt;
	//if a feature selection file is given as parameter (i.e. not all features are calculated)
	if (config.calculateAllFeatures == 0){
		try {
			//get feature selection file
			boost::property_tree::ini_parser::read_ini(config.featureSelectionLocation, pt);
			morph = pt.get("MorphologicalFeatures.CalculateMorphologicalFeat",0);
			std::cout << "calculate morphological features " << morph << std::endl;
			localInt = pt.get("LocalIntensityFeatures.CalculateLocalIntFeat", 0);
			std::cout << "calculate local intensity features " << localInt << std::endl;
			statFeat = pt.get("StatisticalFeatures.CalculateStatFeat", 0);
			std::cout << "calculate statistical features " << statFeat << std::endl;
			intVolFeatures = pt.get("IntensityVolume.CalculateIntensityVolume", 0);
			std::cout << "calculate int volume features " << intVolFeatures << std::endl;
			std::string forLog = "The feature selection file used: " + config.featureSelectionLocation;
			writeLogFile(config.outputFolder, forLog);
			std::cout << "Morphological features are calculated" << std::endl;

		}
		catch (...) {
			std::cout << "Feature selection file was not found." << std::endl;
			exit(EXIT_FAILURE);
		}
			
	
		
		
	}
	
	MorphologicalFeatures<float, 3> morphFeat;
	std::cout << "morph" << std::endl;
	if (morph == a || config.calculateAllFeatures == 1) {
		std::cout << "here" << std::endl;
		morphFeat.calculateAllMorphologicalFeatures(morphFeat, imageAttr, config);
		string forLog = "Morphological features were calculated.";
		writeLogFile(config.outputFolder, forLog);
		std::cout << "Morphological features are calculated" << std::endl;
	}
	if (config.csvOutput == 1 && config.getOneCSVFile == 0) {
		morphFeat.writeCSVFileMorphological(morphFeat, config.outputFolder, config);
		
	}
	else if (config.csvOutput == 1 && config.getOneCSVFile == 1) {
		morphFeat.writeOneFileMorphological(morphFeat, config);
		
	}
	
	LocalIntensityFeatures<float, 3> localIntFeat;
	if (localInt == a || config.calculateAllFeatures == 1) {
		localIntFeat.calculateAllLocalIntensityFeatures(localIntFeat, imageAttr.image, imageAttr.mask, config);
		std::string forLog = "Local intensity features were calculated.";
		writeLogFile(config.outputFolder, forLog);
		std::cout << "Local intensity features are calculated" << std::endl;
	}
	if (config.csvOutput == 1 && config.getOneCSVFile == 0) {
		localIntFeat.writeCSVFileLocalIntensity(localIntFeat, config.outputFolder);
		
	}
	else if ((config.csvOutput == 1 && config.getOneCSVFile == 1) || config.ontologyOutput == 1) {
		localIntFeat.writeOneFileLocalInt(localIntFeat, config);
		
	}
	
	StatisticalFeatures<float, 3> statFeatures;
	if (statFeat == a || config.calculateAllFeatures == 1) {
		statFeatures.calculateAllStatFeatures(statFeatures, imageAttr.vectorOfMatrixElements);
		std::string forLog = "Statistical features were calculated.";
		writeLogFile(config.outputFolder, forLog);
		std::cout << "Statistical Features are calculated" << std::endl;
	}
	if (config.csvOutput == 1 && config.getOneCSVFile == 0) {
		statFeatures.writeCSVFileStatistic(statFeatures, config.outputFolder);
	}
	else if ((config.csvOutput == 1 && config.getOneCSVFile == 1)|| config.ontologyOutput == 1) {
		statFeatures.writeOneFileStatistic(statFeatures, config);
	}
	StatisticalFeatures<float, 3> statFeatures2;
	statFeatures = statFeatures2;
	IntensityVolumeFeatures<float, 3> intVol;
	if ((intVolFeatures == a || config.calculateAllFeatures == 1) && (config.discretizeIVH == 0)) {

		if (config.discretizeIVHSeparated == 0) {
			intVol.calculateAllIntensVolFeatures(intVol, imageAttr.imageMatrix, imageAttr.diffGreyLevels);
		}
	}
	if (config.discretizeIVHSeparated == 0) {
		if (config.csvOutput == 1 && config.getOneCSVFile == 0) {
			intVol.writeCSVFileIntVol(intVol, config.outputFolder);
		}
		else if ((config.csvOutput == 1 && config.getOneCSVFile == 1) || config.ontologyOutput == 1) {
			intVol.writeOneFileIntVol(intVol, config);
		}
		IntensityVolumeFeatures<float, 3> intVol2;
		intVol = intVol2;
		std::string forLog = "Intensity volume features were calculated.";
		writeLogFile(config.outputFolder, forLog);
		std::cout << "Intensity volume features are calculated" << std::endl;
	}
		
}

/*!
In the function calculateRelFeaturesDiscretized, all features that do require interpolation are calculated. \n
If one feature group should not be calculated, this group is skipped from the calculation \n
The feature values are stored in the outputfile set by the user.
*/
void calculateRelFeaturesDiscretized(Image<float, 3> imageAttr, vector<float> spacing, ConfigFile config)
{

	int a = 1;

	boost::property_tree::ptree pt;
	int inthist;
	int intVolFeatures;
	int glcmFeatures2DAVG;
	int glcmFeatures2DDMRG;
	int glcmFeatures2DMRG;
	int glcmFeat2DVMRG;
	int glcmFeat3DAVG;
	int glcmFeat3DMRG;
	int glrlmFeatures2DAVG;
	int glrlmFeatures2DDMRG;
	int glrlmFeat2DMRG;
	int glrlmFeatures2DVMRG;
	int glrlmFeatures3DAVG;
	int glrlmFeatures3DMRG;
	int glszmFeatures2DAVG;
	int glszmFeatures2DMRG;
	int glszmFeatures3D;
	int ngtdmFeatures2DAVG;
	int ngtdmFeat2DMRG;
	int ngtdmFeatures3D;
	int gldzmFeatures2DAVG;
	int gldzmFeatures2D;
	int gldzmFeatures3D;
	int ngldmFeat2DAVG;
	int ngldmFeat2DMRG;
	int ngldmFeat3D;
	//if a feature selection file is given as parameter
	if (config.calculateAllFeatures == 0) {
		try {
			boost::property_tree::ini_parser::read_ini(config.featureSelectionLocation, pt);
			inthist = pt.get("IntensityHistogramFeatures.CalculateIntensityHistogramFeat",0);
			std::cout << "calculate intensity histogram features " << inthist << std::endl;
			intVolFeatures = pt.get("IntensityVolume.CalculateIntensityVolume",0);
			std::cout << "calculate int vol features " << intVolFeatures << std::endl;
			glcmFeatures2DAVG = pt.get("GLCMFeatures2DAVG.CalculateGLCMFeatures2DAVG",0);
			std::cout << "calculate glcm2Davg features " << glcmFeatures2DAVG << std::endl;
			glcmFeatures2DDMRG = pt.get("GLCMFeatures2DDMRG.CalculateGLCMFeatures2DDMRG", 0);
			std::cout << "calculate glcm2DDMRG features " << glcmFeatures2DAVG << std::endl;
			glcmFeatures2DMRG = pt.get("GLCMFeatures2DMRG.CalculateGLCMFeatures2DMRG", 0);
			std::cout << "calculate glcmFeatures2DMRG features " << glcmFeatures2DMRG << std::endl;
			std::cout << "calculate glcm2DMRG features " << glcmFeatures2DMRG << std::endl;
			glcmFeat2DVMRG = pt.get("GLCMFeatures2DVMRG.CalculateGLCMFeatures2DVMRG", 0);
			std::cout << "calculate glcm2DVMRG features " << glcmFeat2DVMRG << std::endl;
			glcmFeat3DAVG = pt.get("GLCMFeatures3DAVG.CalculateGLCMFeatures3DAVG", 0);
			std::cout << "calculate glcm3Davg features " << glcmFeat3DAVG << std::endl;
			glcmFeat3DMRG = pt.get("GLCMFeatures3DMRG.CalculateGLCMFeatures3DMRG", 0);
			std::cout << "calculate glcm3DMRG features " << glcmFeat3DMRG << std::endl;
			glrlmFeatures2DAVG = pt.get("GLRLMFeatures2DAVG.CalculateGLRLMFeatures2DAVG", 0);
			std::cout << "calculate glrlm2DAVG features " << glrlmFeatures2DAVG << std::endl;
			glrlmFeatures2DDMRG = pt.get("GLRLMFeatures2DDMRG.CalculateGLRLMFeatures2DDMRG", 0);
			std::cout << "calculate glrlm2DDMRG features " << glrlmFeatures2DDMRG << std::endl;
			glrlmFeat2DMRG = pt.get("GLRLMFeatures2DMRG.CalculateGLRLMFeatures2DMRG", 0);
			std::cout << "calculate glrlm2DMRG features " << glrlmFeat2DMRG << std::endl;
			glrlmFeatures2DVMRG = pt.get("GLRLMFeatures2DVMRG.CalculateGLRLMFeatures2DVMRG", 0);
			std::cout << "calculate glrlm2DVMRG features " << glrlmFeatures2DVMRG << std::endl;
			glrlmFeatures3DAVG = pt.get("GLRLMFeatures3DAVG.CalculateGLRLMFeatures3DAVG", 0);
			std::cout << "calculate glrlm3DAVG features " << glrlmFeatures3DAVG << std::endl;
			glrlmFeatures3DMRG = pt.get("GLRLMFeatures3DMRG.CalculateGLRLMFeatures3DMRG", 0);
			std::cout << "calculate glcm2Davg features " << glrlmFeatures3DMRG << std::endl;
			glszmFeatures2DAVG = pt.get("GLSZMFeatures2DAVG.CalculateGLSZMFeatures2DAVG", 0);
			std::cout << "calculate glszm2DAVG features " << glszmFeatures2DAVG << std::endl;
			glszmFeatures2DMRG = pt.get("GLSZMFeatures2DMRG.CalculateGLSZMFeatures2DMRG", 0);
			std::cout << "calculate glszm2DMRG features " << glszmFeatures2DMRG << std::endl;
			glszmFeatures3D = pt.get("GLSZMFeatures3D.CalculateGLSZMFeatures3D", 0);
			std::cout << "calculate glszm3D features " << glszmFeatures3D << std::endl;
			ngtdmFeatures2DAVG = pt.get("NGTDMFeatures2DAVG.CalculateNGTDMFeatures2DAVG", 0);
			std::cout << "calculate ngtdm2DAVG features " << ngtdmFeatures2DAVG << std::endl;
			ngtdmFeat2DMRG = pt.get("NGTDMFeatures2DMRG.CalculateNGTDMFeatures2DMRG", 0);
			std::cout << "calculate ngtdm2DMRG features " << ngtdmFeat2DMRG << std::endl;
			ngtdmFeatures3D = pt.get("NGTDMFeatures3D.CalculateNGTDMFeatures3D", 0);
			std::cout << "calculate ngtdm3D features " << ngtdmFeatures3D << std::endl;
			gldzmFeatures2DAVG = pt.get("GLDZMFeatures2DAVG.CalculateGLDZMFeatures2DAVG", 0);
			std::cout << "calculate gldzm2DAVG features " << gldzmFeatures2DAVG << std::endl;
			gldzmFeatures2D = pt.get("GLDZMFeatures2D.CalculateGLDZMFeatures2D", 0);
			std::cout << "calculate gldzm2D features " << gldzmFeatures2D << std::endl;
			gldzmFeatures3D = pt.get("GLDZMFeatures3D.CalculateGLDZMFeatures3D", 0);
			std::cout << "calculate gldzm3D features " << gldzmFeatures3D << std::endl;
			ngldmFeat2DAVG = pt.get("NGLDMFeatures2DAVG.CalculateNGLDMFeatures2DAVG", 0);
			std::cout << "calculate ngldm2DAVG features " << ngldmFeat2DAVG << std::endl;
			ngldmFeat2DMRG = pt.get("NGLDMFeatures2DMRG.CalculateNGLDMFeatures2DMRG", 0);
			std::cout << "calculate ngldm2DMRG features " << ngldmFeat2DMRG << std::endl;
			ngldmFeat3D = pt.get("NGLDMFeatures3D.CalculateNGLDMFeatures3D",0);
			std::cout << "calculate ngldm3D features " << ngldmFeat3D << std::endl;
		}
		catch (...) {
			std::cout << "Feature selection file was not found. Please check the configuration in the config.ini file." << std::endl;
			exit(EXIT_FAILURE);
		}
	}
	IntensityVolumeFeatures<float, 3> intVol;
	if ((intVolFeatures == a || config.calculateAllFeatures == 1) && (config.discretizeIVHSeparated == 1 && config.discretizeIVH == 1)) {


		intVol.calculateAllIntensVolFeatures(intVol, imageAttr.imageMatrixIVH, imageAttr.diffGreyLevels);
	}
	if(config.discretizeIVHSeparated == 1 && config.discretizeIVH == 1){
		if (config.csvOutput == 1 && config.getOneCSVFile == 0) {
			intVol.writeCSVFileIntVol(intVol, config.outputFolder);
		}
		else if ((config.csvOutput == 1 && config.getOneCSVFile == 1) || config.ontologyOutput == 1) {
			intVol.writeOneFileIntVol(intVol, config);
		}
		std::string forLog = "Intensity volume features were calculated.";
		writeLogFile(config.outputFolder, forLog);
		std::cout << "Intensity volume features are calculated" << std::endl;
	}

	if ((intVolFeatures == a || config.calculateAllFeatures == 1) && (config.discretizeIVHSeparated == 0 && config.discretizeIVH == 1)) {

		intVol.calculateAllIntensVolFeatures(intVol, imageAttr.imageMatrix, imageAttr.diffGreyLevels);
	}
	if(config.discretizeIVHSeparated == 0 && config.discretizeIVH == 1){
		if (config.csvOutput == 1 && config.getOneCSVFile == 0) {
			intVol.writeCSVFileIntVol(intVol, config.outputFolder);
		}
		else if ((config.csvOutput == 1 && config.getOneCSVFile == 1) || config.ontologyOutput == 1) {
			intVol.writeOneFileIntVol(intVol, config);
		}
		std::string forLog = "Intensity volume features were calculated.";
		writeLogFile(config.outputFolder, forLog);
		std::cout << "Intensity volume features are calculated" << std::endl;
	}
	IntensityVolumeFeatures<float, 3> intVol2;
	intVol = intVol2;
   IntensityHistogram<float, 3> intensityHist;
   if (inthist == a || config.calculateAllFeatures == 1) {

	   intensityHist.calculateAllIntFeatures(intensityHist, imageAttr.imageMatrix, imageAttr.vectorOfMatrixElements, imageAttr.diffGreyLevels);
	   std::string forLog = "Intensity histogram features were calculated.";
	   writeLogFile(config.outputFolder, forLog);
	   std::cout << "Intensity histogram features are calculated" << std::endl;

   }
	if (config.csvOutput == 1 && config.getOneCSVFile == 0) {
			intensityHist.writeCSVFileIntensity(intensityHist, config.outputFolder);
	}
	else if ((config.csvOutput == 1 && config.getOneCSVFile == 1) || config.ontologyOutput == 1) {
		intensityHist.writeOneFileIntensity(intensityHist, config);
	}
	IntensityHistogram<float, 3> intensityHist2;
	intensityHist = intensityHist2;
   
   
    float maxIntensity = float(*max_element(imageAttr.vectorOfMatrixElements.begin(), imageAttr.vectorOfMatrixElements.end()));
	GLCMFeatures2DAVG<float, 3> glcm2DAVG;
	if (glcmFeatures2DAVG == a || config.calculateAllFeatures == 1) {

		glcm2DAVG.calculateAllGLCMFeatures2DAVG(glcm2DAVG, imageAttr.imageMatrix, maxIntensity);
		std::string forLog = "GLCM2DAVG features were calculated.";
		writeLogFile(config.outputFolder, forLog);
	}
	if (config.csvOutput == 1 && config.getOneCSVFile == 0) {
		glcm2DAVG.writeCSVFileGLCM2DAVG(glcm2DAVG, config.outputFolder);
	}
	else if ((config.csvOutput == 1 && config.getOneCSVFile == 1) || config.ontologyOutput == 1) {
		glcm2DAVG.writeOneFileGLCM2DAVG(glcm2DAVG, config, config.featureParameterSpaceNr);
	}
	GLCMFeatures2DAVG<float, 3> glcm2DAVG2;
	glcm2DAVG = glcm2DAVG2;
	GLCMFeatures2DDMRG<float, 3> glcm2DDMRG;
	if (glcmFeatures2DDMRG == a || config.calculateAllFeatures == 1) {


		glcm2DDMRG.calculateAllGLCMFeatures2DDMRG(glcm2DDMRG, imageAttr.imageMatrix, maxIntensity);
		std::string forLog = "GLCM2DDMRG features were calculated.";
		writeLogFile(config.outputFolder, forLog);
	}
	if (config.csvOutput == 1 && config.getOneCSVFile == 0) {
		glcm2DDMRG.writeCSVFileGLCM2DDMRG(glcm2DDMRG, config.outputFolder);
	}
	else if ((config.csvOutput == 1 && config.getOneCSVFile == 1) || config.ontologyOutput == 1) {
		glcm2DDMRG.writeOneFileGLCM2DDMRG(glcm2DDMRG, config, config.featureParameterSpaceNr);
	}
	GLCMFeatures2DDMRG<float, 3> glcm2DDMRG2;
	glcm2DDMRG = glcm2DDMRG2;
	GLCMFeatures2DMRG<float, 3> glcm2DMRG;
	if (glcmFeatures2DMRG == a || config.calculateAllFeatures == 1) {

		glcm2DMRG.calculateAllGLCMFeatures2DMRG(glcm2DMRG, imageAttr.imageMatrix, maxIntensity, spacing, config);
		std::string forLog = "GLCM2DMRG features were calculated.";
		writeLogFile(config.outputFolder, forLog);
	}
	if (config.csvOutput == 1 && config.getOneCSVFile == 0) {
		glcm2DMRG.writeCSVFileGLCM2DMRG(glcm2DMRG, config.outputFolder);
	}
	else if ((config.csvOutput == 1 && config.getOneCSVFile == 1) || config.ontologyOutput == 1) {
		glcm2DMRG.writeOneFileGLCM2DMRG(glcm2DMRG, config, config.featureParameterSpaceNr);
	}
	GLCMFeatures2DMRG<float, 3> glcm2DMRG2;
	glcm2DMRG = glcm2DMRG2;
	
	GLCMFeatures2DVMRG<float, 3> glcm2DVMRG;
	if (glcmFeat2DVMRG == a || config.calculateAllFeatures == 1) {

		glcm2DVMRG.calculateAllGLCMFeatures2DVMRG(glcm2DVMRG, imageAttr.imageMatrix, maxIntensity, spacing, config);
		std::string forLog = "GLCM2DVMRG features were calculated.";
		writeLogFile(config.outputFolder, forLog);

	}
	if (config.csvOutput == 1 && config.getOneCSVFile == 0) {
		glcm2DVMRG.writeCSVFileGLCM2DVMRG(glcm2DVMRG, config.outputFolder);
	}
	else if ((config.csvOutput == 1 && config.getOneCSVFile == 1) || config.ontologyOutput == 1) {
		glcm2DVMRG.writeOneFileGLCM2DVMRG(glcm2DVMRG, config, config.featureParameterSpaceNr);
	}
	GLCMFeatures2DVMRG<float, 3> glcm2DVMRG2;
	glcm2DVMRG = glcm2DVMRG2;
	GLCMFeatures3DAVG<float, 3> glcmFeat3DAVGFeat;
	if (glcmFeat3DAVG == a || config.calculateAllFeatures == 1) {

		glcmFeat3DAVGFeat.calculateAllGLCMFeatures3DAVG(glcmFeat3DAVGFeat, imageAttr.imageMatrix, maxIntensity);
		std::string forLog = "GLCM3DAVG features were calculated.";
		writeLogFile(config.outputFolder, forLog);
	}
	if (config.csvOutput == 1 && config.getOneCSVFile == 0) {
		glcmFeat3DAVGFeat.writeCSVFileGLCM3DAVG(glcmFeat3DAVGFeat, config.outputFolder);
	}
	else if ((config.csvOutput == 1 && config.getOneCSVFile == 1) || config.ontologyOutput == 1) {
		glcmFeat3DAVGFeat.writeOneFileGLCM3DAVG(glcmFeat3DAVGFeat, config, config.featureParameterSpaceNr);
	}
	GLCMFeatures3DAVG<float, 3> glcmFeat3DAVGFeat2;
	glcmFeat3DAVGFeat = glcmFeat3DAVGFeat2;
	GLCMFeatures3DMRG<float, 3> glcm3DMRG;
	if (glcmFeat3DMRG == a || config.calculateAllFeatures == 1) {

		glcm3DMRG.calculateAllGLCMFeatures3DMRG(glcm3DMRG, imageAttr.imageMatrix, maxIntensity, spacing, config);
		std::string forLog = "GLCM3DMRG features were calculated.";
		writeLogFile(config.outputFolder, forLog);
	}
	if (config.csvOutput == 1 && config.getOneCSVFile == 0) {
		glcm3DMRG.writeCSVFileGLCM3DMRG(glcm3DMRG, config.outputFolder);
	}
	else if ((config.csvOutput == 1 && config.getOneCSVFile == 1) || config.ontologyOutput == 1) {
		glcm3DMRG.writeOneFileGLCM3DMRG(glcm3DMRG, config, config.featureParameterSpaceNr);
	}
	GLCMFeatures3DMRG<float, 3> glcm3DMRG2;
	glcm3DMRG = glcm3DMRG2;
	std::cout << "GLCM features are calculated" << std::endl;
	
	GLRLMFeatures2DAVG<float, 3> glrlm2DAVG;
	if (glrlmFeatures2DAVG == a || config.calculateAllFeatures == 1) {

		glrlm2DAVG.calculateAllGLRLMFeatures2DAVG(glrlm2DAVG, imageAttr.imageMatrix, imageAttr.diffGreyLevels, config);
		std::string forLog = "GLRLM2DAVG features were calculated.";
		writeLogFile(config.outputFolder, forLog);

	}
	if (config.csvOutput == 1 && config.getOneCSVFile == 0) {
		glrlm2DAVG.writeCSVFileGLRLM2DAVG(glrlm2DAVG, config.outputFolder);
	}
	else if ((config.csvOutput == 1 && config.getOneCSVFile == 1) || config.ontologyOutput == 1) {
		glrlm2DAVG.writeOneFileGLRLM2DAVG(glrlm2DAVG, config, config.featureParameterSpaceNr);
	}
	GLRLMFeatures2DAVG<float, 3> glrlm2DAVG2;
	glrlm2DAVG = glrlm2DAVG2;
	GLRLMFEATURES2DDMRG<float, 3> glrlm2DDMRG;
	if (glrlmFeatures2DDMRG == a || config.calculateAllFeatures == 1) {

		glrlm2DDMRG.calculateAllGLRLMFeatures2DDMRG(glrlm2DDMRG, imageAttr.imageMatrix, imageAttr.diffGreyLevels, spacing, config);
		std::string forLog = "GLRLM2DAVG features were calculated.";
		writeLogFile(config.outputFolder, forLog);

	}
	if (config.csvOutput == 1 && config.getOneCSVFile == 0) {
		glrlm2DDMRG.writeCSVFileGLRLM2DDMRG(glrlm2DDMRG, config.outputFolder);
	}
	else if ((config.csvOutput == 1 && config.getOneCSVFile == 1) || config.ontologyOutput == 1) {
		glrlm2DDMRG.writeOneFileGLRLM2DDMRG(glrlm2DDMRG, config, config.featureParameterSpaceNr);
	}
	GLRLMFEATURES2DDMRG<float, 3> glrlm2DDMRG2;
	glrlm2DDMRG = glrlm2DDMRG2;
	GLRLMFeatures2DMRG<float, 3> glrlm2DMRG;
	if (glrlmFeat2DMRG == a || config.calculateAllFeatures == 1) {

		glrlm2DMRG.calculateAllGLRLMFeatures2DMRG(glrlm2DMRG, imageAttr.imageMatrix, imageAttr.diffGreyLevels, spacing, config);
		std::string forLog = "GLRLM2DMRG features were calculated.";
		writeLogFile(config.outputFolder, forLog);

	}
	if (config.csvOutput == 1 && config.getOneCSVFile == 0) {
		glrlm2DMRG.writeCSVFileGLRLM2DMRG(glrlm2DMRG, config.outputFolder);
	}
	else if ((config.csvOutput == 1 && config.getOneCSVFile == 1) || config.ontologyOutput == 1) {
		glrlm2DMRG.writeOneFileGLRLM2DMRG(glrlm2DMRG, config, config.featureParameterSpaceNr);
	}
	GLRLMFeatures2DMRG<float, 3> glrlm2DMRG2;
	glrlm2DMRG = glrlm2DMRG2;
	GLRLMFeatures2DVMRG<float, 3> glrlm2DVMRG;
	if (glrlmFeatures2DVMRG == a || config.calculateAllFeatures == 1) {

		glrlm2DVMRG.calculateAllGLRLMFeatures2DVMRG(glrlm2DVMRG, imageAttr.imageMatrix, imageAttr.diffGreyLevels, imageAttr.vectorOfMatrixElements, spacing, config);
		std::string forLog = "GLRLM2DVMRG features were calculated.";
		writeLogFile(config.outputFolder, forLog);

	}
	if (config.csvOutput == 1 && config.getOneCSVFile == 0) {
		glrlm2DVMRG.writeCSVFileGLRLM2DVMRG(glrlm2DVMRG, config.outputFolder);
	}
	else if ((config.csvOutput == 1 && config.getOneCSVFile == 1) || config.ontologyOutput == 1) {
		glrlm2DVMRG.writeOneFileGLRLM2DVMRG(glrlm2DVMRG, config, config.featureParameterSpaceNr);
	}
	GLRLMFeatures2DVMRG<float, 3> glrlm2DVMRG2;
	glrlm2DVMRG = glrlm2DVMRG2;
	GLRLMFeatures3DAVG<float, 3> glrlm3DAVG;
	if (glrlmFeatures3DAVG == a || config.calculateAllFeatures == 1) {

		glrlm3DAVG.calculateAllGLRLMFeatures3DAVG(glrlm3DAVG, imageAttr.imageMatrix, imageAttr.diffGreyLevels, imageAttr.vectorOfMatrixElements, config);
		std::string forLog = "GLRLM3DAVG features were calculated.";
		writeLogFile(config.outputFolder, forLog);

	}
	if (config.csvOutput == 1 && config.getOneCSVFile == 0) {
		glrlm3DAVG.writeCSVFileGLRLM3DAVG(glrlm3DAVG, config.outputFolder);
	}
	else if ((config.csvOutput == 1 && config.getOneCSVFile == 1) || config.ontologyOutput == 1) {
		glrlm3DAVG.writeOneFileGLRLM3DAVG(glrlm3DAVG, config, config.featureParameterSpaceNr);
	}
	GLRLMFeatures3DAVG<float, 3> glrlm3DAVG2;
	glrlm3DAVG = glrlm3DAVG2;
	GLRLMFeatures3D<float, 3> glrlm3DMRG;
	if (glrlmFeatures3DMRG == a || config.calculateAllFeatures == 1) {

		glrlm3DMRG.calculateAllGLRLMFeatures3D(glrlm3DMRG, imageAttr.imageMatrix, imageAttr.diffGreyLevels, imageAttr.vectorOfMatrixElements, spacing, config);
		std::string forLog = "GLRLM3DMRG features were calculated.";
		writeLogFile(config.outputFolder, forLog);

	}
	if (config.csvOutput == 1 && config.getOneCSVFile == 0) {
		glrlm3DMRG.writeCSVFileGLRLM3D(glrlm3DMRG, config.outputFolder);
	}
	else if ((config.csvOutput == 1 && config.getOneCSVFile == 1) || config.ontologyOutput == 1) {
		glrlm3DMRG.writeOneFileGLRLM3D(glrlm3DMRG, config, config.featureParameterSpaceNr);
	}
	GLRLMFeatures3D<float, 3> glrlm3DMRG2;
	glrlm3DMRG = glrlm3DMRG2;
	std::cout << "GLRLM features are calculated" << std::endl;
	GLSZMFeatures2DAVG<float, 3> glszm2DAVG;
	if (glszmFeatures2DAVG == a || config.calculateAllFeatures == 1) {

		glszm2DAVG.calculateAllGLSZMFeatures2DAVG(glszm2DAVG, imageAttr.imageMatrix, imageAttr.diffGreyLevels, config);
		std::string forLog = "GLSZM2DAVG features were calculated.";
		writeLogFile(config.outputFolder, forLog);

	}
	if (config.csvOutput == 1 && config.getOneCSVFile == 0) {
		glszm2DAVG.writeCSVFileGLSZM2DAVG(glszm2DAVG, config.outputFolder);
	}
	else if ((config.csvOutput == 1 && config.getOneCSVFile == 1) || config.ontologyOutput == 1) {
		glszm2DAVG.writeOneFileGLSZM2DAVG(glszm2DAVG, config, config.featureParameterSpaceNr);
	}
	GLSZMFeatures2DAVG<float, 3> glszm2DAVG2;
	glszm2DAVG = glszm2DAVG2;
	GLSZMFeatures2DMRG<float, 3> glszm2D;
	if (glszmFeatures2DMRG == a || config.calculateAllFeatures == 1) {

		glszm2D.calculateAllGLSZMFeatures2DMRG(glszm2D, imageAttr.imageMatrix, imageAttr.diffGreyLevels, imageAttr.vectorOfMatrixElements, config);
		std::string forLog = "GLSZM2DMRG features were calculated.";
		writeLogFile(config.outputFolder, forLog);

	}
	if (config.csvOutput == 1 && config.getOneCSVFile == 0) {
		glszm2D.writeCSVFileGLSZM(glszm2D, config.outputFolder);
	}
	else if ((config.csvOutput == 1 && config.getOneCSVFile == 1) || config.ontologyOutput == 1) {
		glszm2D.writeOneFileGLSZM(glszm2D, config, config.featureParameterSpaceNr);
	}
	GLSZMFeatures2DMRG<float, 3> glszm2D2;
	glszm2D = glszm2D2;
	GLSZMFeatures3D<float, 3> glszm3D;
	if (glszmFeatures3D == a || config.calculateAllFeatures == 1) {

		glszm3D.calculateAllGLSZMFeatures3D(glszm3D, imageAttr, config);
		std::string forLog = "GLSZM3D features were calculated.";
		writeLogFile(config.outputFolder, forLog);

	}
	if (config.csvOutput == 1 && config.getOneCSVFile == 0) {
		glszm3D.writeCSVFileGLSZM3D(glszm3D, config.outputFolder);
	}
	else if ((config.csvOutput == 1 && config.getOneCSVFile == 1) || config.ontologyOutput == 1) {
		glszm3D.writeOneFileGLSZM3D(glszm3D, config, config.featureParameterSpaceNr);
	}
	GLSZMFeatures3D<float, 3> glszm3D2;
	glszm3D = glszm3D2;

	std::cout << "GLSZM features are calculated" << std::endl;	

	boost::multi_array<float, 3> ngtdm2D(boost::extents[imageAttr.imageMatrix.shape()[0]][imageAttr.imageMatrix.shape()[1]][imageAttr.imageMatrix.shape()[2]]);
	boost::multi_array<float, 3> nrNeighborMatrix(boost::extents[imageAttr.imageMatrix.shape()[0] + 1][imageAttr.imageMatrix.shape()[1]][imageAttr.imageMatrix.shape()[2]]);

	int sizeGreyLevels = (imageAttr.diffGreyLevels).size();
	//define the NGLDMarices; col.size=9 because we have 8 neighbors
	boost::multi_array<float, 3> NGLDMatrix(boost::extents[sizeGreyLevels][9][imageAttr.imageMatrix.shape()[2]]);
	boost::multi_array<float, 3> ngtdm3DMatrix(boost::extents[imageAttr.imageMatrix.shape()[0]][imageAttr.imageMatrix.shape()[1]][imageAttr.imageMatrix.shape()[2]]);
	boost::multi_array<float, 2> ngldm3DMatrixSum(boost::extents[sizeGreyLevels][27]);

	if (ngtdmFeatures2DAVG == a || config.calculateAllFeatures == 1 || ngtdmFeat2DMRG == a || ngldmFeat2DMRG == a || ngldmFeat2DAVG == a) {
		getNeighborhoodMatrix2D(imageAttr, ngtdm2D, spacing, config);
		getNeighborhoodMatrix2DNGLDM(imageAttr, NGLDMatrix, spacing, config);
	}

	if (ngtdmFeatures3D == a || config.calculateAllFeatures == 1 || ngldmFeat3D == a) {
		getNeighborhoodMatrix3D_convolution(imageAttr, ngtdm3DMatrix, spacing, config);
		getNGLDMatrix3D_convolution(imageAttr, ngldm3DMatrixSum, spacing, config);
	}
	
			
	
	NGTDM2DAVG<float, 3> ngtdm2Davg;
	if (ngtdmFeatures2DAVG == a || config.calculateAllFeatures == 1) {

		ngtdm2Davg.calculateAllNGTDMFeatures2DAVG(ngtdm2Davg, imageAttr, ngtdm2D, spacing, config);
		std::string forLog = "NGTDM2DAVG features were calculated.";
		writeLogFile(config.outputFolder, forLog);

	}
	if (config.csvOutput == 1 && config.getOneCSVFile == 0) {
		ngtdm2Davg.writeCSVFileNGTDM2DAVG(ngtdm2Davg, config.outputFolder);
	}
	else if ((config.csvOutput == 1 && config.getOneCSVFile == 1) || config.ontologyOutput == 1) {
		ngtdm2Davg.writeOneFileNGTDM2DAVG(ngtdm2Davg, config, config.featureParameterSpaceNr);
	}
	NGTDM2DAVG<float, 3> NGTDM2DAVG2;
	ngtdm2Davg = NGTDM2DAVG2;
	NGTDMFeatures2DMRG<float, 3> ngtdm2DMRG;
	if (ngtdmFeat2DMRG == a || config.calculateAllFeatures == 1) {

		ngtdm2DMRG.calculateAllNGTDMFeatures2DMRG(ngtdm2DMRG, imageAttr, ngtdm2D, spacing, config);
		std::string forLog = "NGTDM2DMRG features were calculated.";
		writeLogFile(config.outputFolder, forLog);

	}
	if (config.csvOutput == 1 && config.getOneCSVFile == 0) {
		ngtdm2DMRG.writeCSVFileNGTDM(ngtdm2DMRG, config.outputFolder);
	}
	else if ((config.csvOutput == 1 && config.getOneCSVFile == 1) || config.ontologyOutput == 1) {
		ngtdm2DMRG.writeOneFileNGTDM(ngtdm2DMRG, config, config.featureParameterSpaceNr);
	}
	NGTDMFeatures2DMRG<float, 3> ngtdm2DMRG2;
	ngtdm2DMRG = ngtdm2DMRG2;
	NGTDMFeatures3D<float, 3> ngtdm3D;
	if (ngtdmFeatures3D == a || config.calculateAllFeatures == 1) {

		ngtdm3D.calculateAllNGTDMFeatures3D(ngtdm3D, ngtdm3DMatrix, imageAttr, spacing, config);
		std::string forLog = "NGTDM3D features were calculated.";
		writeLogFile(config.outputFolder, forLog);
	}
	if (config.csvOutput == 1 && config.getOneCSVFile == 0) {
		ngtdm3D.writeCSVFileNGTDM3D(ngtdm3D, config.outputFolder);
	}
	else if ((config.csvOutput == 1 && config.getOneCSVFile == 1) || config.ontologyOutput == 1) {
		ngtdm3D.writeOneFileNGTDM3D(ngtdm3D, config, config.featureParameterSpaceNr);
	}
	std::cout << "NGTDM features are calculated" << std::endl;
	NGTDMFeatures3D<float, 3> ngtdm3D2;
	ngtdm3D = ngtdm3D2;
	boost::multi_array<float, 3> ngtdm3DMatrix2(boost::extents[0][0][0]);
	ngtdm3DMatrix = ngtdm3DMatrix2;

	GLDZMFeatures2DAVG<float, 3> gldzm2DAVG;
	boost::multi_array<float, 3> distanceMap(boost::extents[imageAttr.imageMatrix.shape()[0]][imageAttr.imageMatrix.shape()[1]][imageAttr.imageMatrix.shape()[2]]);
	if (config.useReSegmentation == 1 || config.excludeOutliers == 1) {
		gldzm2DAVG.generateDistanceMap(imageAttr.imageMatrixOriginal, imageAttr, distanceMap, config);
	}
	else {
		gldzm2DAVG.generateDistanceMap(imageAttr.imageMatrix, imageAttr, distanceMap, config);
	}


	if (gldzmFeatures2DAVG == a || config.calculateAllFeatures == 1) {

		gldzm2DAVG.calculateAllGLDZMFeatures2DAVG(gldzm2DAVG, imageAttr, distanceMap, config);
		std::string forLog = "GLDZM2DAVG features were calculated.";
		writeLogFile(config.outputFolder, forLog);

	}
	if (config.csvOutput == 1 && config.getOneCSVFile == 0) {
		gldzm2DAVG.writeCSVFileGLDZM2DAVG(gldzm2DAVG, config.outputFolder);
	}
	else if ((config.csvOutput == 1 && config.getOneCSVFile == 1) || config.ontologyOutput == 1) {
		gldzm2DAVG.writeOneFileGLDZM2DAVG(gldzm2DAVG, config, config.featureParameterSpaceNr);
	}
	GLDZMFeatures2DAVG<float, 3> gldzm2DAVG2;
	gldzm2DAVG = gldzm2DAVG2;
	GLDZMFeatures2D<float, 3> gldzm2D;
	if (gldzmFeatures2D == a || config.calculateAllFeatures == 1) {

		gldzm2D.calculateAllGLDZMFeatures2D(gldzm2D, distanceMap, imageAttr, config);
		std::string forLog = "GLDZM2DMRG features were calculated.";
		writeLogFile(config.outputFolder, forLog);

	}
	if (config.csvOutput == 1 && config.getOneCSVFile == 0) {
		gldzm2D.writeCSVFileGLDZM(gldzm2D, config.outputFolder);
	}
	else if ((config.csvOutput == 1 && config.getOneCSVFile == 1) || config.ontologyOutput == 1) {
		gldzm2D.writeOneFileGLDZM(gldzm2D, config, config.featureParameterSpaceNr);
	}
	GLDZMFeatures2D<float, 3> gldzm2D2;
	gldzm2D = gldzm2D2;
	GLDZMFeatures3D<float, 3> gldzm3D;
	if (gldzmFeatures3D == a || config.calculateAllFeatures == 1) {

		gldzm3D.calculateAllGLDZMFeatures3D(gldzm3D, distanceMap, imageAttr, config);
		std::string forLog = "GLDZM3D features were calculated.";
		writeLogFile(config.outputFolder, forLog);

	}
	if (config.csvOutput == 1 && config.getOneCSVFile == 0) {
		gldzm3D.writeCSVFileGLDZM3D(gldzm3D, config.outputFolder);
	}
	else if ((config.csvOutput == 1 && config.getOneCSVFile == 1) || config.ontologyOutput == 1) {
		gldzm3D.writeOneFileGLDZM3D(gldzm3D, config, config.featureParameterSpaceNr);
	}
	GLDZMFeatures3D<float, 3> gldzm3D2;
	gldzm3D = gldzm3D2;
	std::cout << "GLDZM features are calculated" << std::endl;
	NGLDMFeatures2DAVG<float, 3> ngldm2DAVG;
	if (ngldmFeat2DAVG == a || config.calculateAllFeatures == 1) {

		ngldm2DAVG.calculateAllNGLDMFeatures2DAVG(ngldm2DAVG, imageAttr, NGLDMatrix, config);
		std::string forLog = "NGLDM2DAVG features were calculated.";
		writeLogFile(config.outputFolder, forLog);
	}
	if (config.csvOutput == 1 && config.getOneCSVFile == 0) {
		ngldm2DAVG.writeCSVFileNGLDM2DAVG(ngldm2DAVG, config.outputFolder);
	}
	else if ((config.csvOutput == 1 && config.getOneCSVFile == 1) || config.ontologyOutput == 1) {
		ngldm2DAVG.writeOneFileNGLDM2DAVG(ngldm2DAVG, config, config.featureParameterSpaceNr);
	}
	
	NGLDMFeatures2DMRG<float, 3> ngldm2DMRG;
	if (ngldmFeat2DMRG == a || config.calculateAllFeatures == 1) {

		ngldm2DMRG.calculateAllNGLDMFeatures2DMRG(ngldm2DMRG, imageAttr, NGLDMatrix, config);
		std::string forLog = "NGLDM2DMRG features were calculated.";
		writeLogFile(config.outputFolder, forLog);

	}
	if (config.csvOutput == 1 && config.getOneCSVFile == 0) {
		ngldm2DMRG.writeCSVFileNGLDM2DMRG(ngldm2DMRG, config.outputFolder);
	}
	else if ((config.csvOutput == 1 && config.getOneCSVFile == 1) || config.ontologyOutput == 1) {
		ngldm2DMRG.writeOneFileNGLDM2DMRG(ngldm2DMRG, config, config.featureParameterSpaceNr);
	}
	
	NGLDMFeatures3D<float, 3> ngldm3D;
	if (ngldmFeat3D == a || config.calculateAllFeatures == 1) {

		ngldm3D.calculateAllNGLDMFeatures3D(ngldm3D, ngldm3DMatrixSum, imageAttr, config);
		std::string forLog = "NGLDM3D features were calculated.";
		writeLogFile(config.outputFolder, forLog);
	}
	if (config.csvOutput == 1 && config.getOneCSVFile == 0) {
		ngldm3D.writeCSVFileNGLDM3D(ngldm3D, config.outputFolder);
	}
	else if ((config.csvOutput == 1 && config.getOneCSVFile == 1) || config.ontologyOutput == 1) {
		ngldm3D.writeOneFileNGLDM3D(ngldm3D, config, config.featureParameterSpaceNr);
	}
		
	std::cout << "NGLDM features are calculated" << std::endl;

}

void writeLogFile(string logFileName, std::string &text) {
	logFileName = logFileName + ".log";
	std::ofstream log_file(logFileName, std::ios_base::out | std::ios_base::app);
	log_file << text << std::endl;
}
