//convert an array to an ITK image
ImageType::Pointer converArray2Image(float *imageArray, unsigned int* dim, float *voxelSize) {
	
	ImageType::Pointer finalImage;
	//write the array in image
	ImportFilterType::Pointer importFilter = ImportFilterType::New();
	ImportFilterType::SizeType size;
	//set image size
	size[0] = dim[0];
	size[1] = dim[1];
	size[2] = dim[2];

	int nrVoxelsPET = dim[0] * dim[1] * dim[2];
	//start with importing the image
	ImportFilterType::IndexType start;
	start.Fill(0);
	//set image region
	ImportFilterType::RegionType region;
	region.SetIndex(start);
	region.SetSize(size);
	importFilter->SetRegion(region);

	float centerX = float((voxelSize[0] * dim[0]) / 2.0);
	float centerY = float((voxelSize[1] * dim[1]) / 2.0);

	float centerZ = float((voxelSize[2] * dim[2]) / 2.0);
	const itk::SpacePrecisionType origin[3] = { centerX, centerY, -centerZ };
	const itk::SpacePrecisionType spacing[3] = { voxelSize[0], voxelSize[1], voxelSize[2] };
	importFilter->SetOrigin(origin);
	importFilter->SetSpacing(spacing);
	const bool importImageFilterWillOwnTheBuffer = true;
	importFilter->SetImportPointer(imageArray, nrVoxelsPET, importImageFilterWillOwnTheBuffer);
	importFilter->Update();
	finalImage = importFilter->GetOutput();
	finalImage->Register();

	using FlipImageFilterType = itk::FlipImageFilter<ImageType>;
	FlipImageFilterType::Pointer flipFilter = FlipImageFilterType::New();
	flipFilter->SetInput(finalImage);
	FlipImageFilterType::FlipAxesArrayType flipAxes;
	flipAxes[0] = false;
	flipAxes[1] = true;
	flipAxes[2] = false;
	flipFilter->SetFlipAxes(flipAxes);
	flipFilter->Update();
	return flipFilter->GetOutput();
}



//the project file of the accurate tool is read in
ImageType::Pointer readPrjFilePET(string prjPath, string imageType, float smoothingKernel, unsigned int(&dimPET)[3], float(&voxelSize)[3]) {
	//help array to read in the integer byte
	unsigned int dimCT[3];
	float voxelSizeCT[3];
	float halfLife, volumeScale, frameScale;
	ImageType::Pointer PETimage;
	//read in prj file
	ifstream prjfile;
	prjfile.open(prjPath, ios::binary);
	if (prjfile.is_open() == false) {
		std::cout << "Cannot open project file\n";
	}
	else {
		//get the image dimensions, voxel sizes and half life of the tracer
		getImageDimension(prjfile, dimPET);
		getVoxelSize(prjfile, voxelSize);
		getHalfLifeScale(prjfile, halfLife, volumeScale, frameScale);
		getImageDimension(prjfile, dimCT);
		getVoxelSize(prjfile, voxelSizeCT);
		int nrVoxelsPET = int(dimPET[0] * dimPET[1] * dimPET[2]);
		vector<float> petVector(nrVoxelsPET);
		vector<float> ctVector(nrVoxelsPET);
		getImageValues(prjfile, petVector);
		
		
		//if image is PET image get array with PET values
		if (imageType == "PET") {
			float *arr = &petVector[0];
			PETimage = converArray2Image(arr, dimPET, voxelSize);
			PETimage->Update();
			vector<float>().swap(petVector);
			arr = nullptr;
		}

		//else get arrat with CT values
		else if (imageType == "CT") {
			getImageValues(prjfile, ctVector);
			float *arr = &ctVector[0];
			PETimage = converArray2Image(arr, dimCT, voxelSizeCT);
			PETimage->Update();
		}
		else {
			std::cout << "Unknown image type. Please check the image type in the config file" << std::endl;
		}
		itk::FixedArray<bool, 3> flipAxes;
		flipAxes[0] = false;
		flipAxes[1] = false;
		flipAxes[2] = false;
		typedef itk::FlipImageFilter <ImageType> FlipImageFilterType;
		FlipImageFilterType::Pointer flipFilter = FlipImageFilterType::New();
		flipFilter->SetInput(PETimage);
		flipFilter->SetFlipAxes(flipAxes);
		flipFilter->Update();
		PETimage = flipFilter->GetOutput();
		PETimage->Update();
		if (smoothingKernel > 0) {

			PETimage = smoothImage(PETimage, smoothingKernel);
		}
		prjfile.close();
	}
	return PETimage;
}

//to do: read in as array
void getHalfLifeScale(ifstream &inFile, float &halfLife, float &volumeScale, float &frameScale) {
	inFile.read((char*)&halfLife, sizeof(float));
	inFile.read((char*)&volumeScale, sizeof(float));
	inFile.read((char*)&frameScale, sizeof(float));
}

void getImageValues(ifstream &inFile, vector<float> &imageArray) {
	for (int i = 0; i < boost::size(imageArray); i++) {
		inFile.read((char*)&imageArray[i], sizeof(float));
	}
}
//get the dimension of the image
void getImageDimension(ifstream &inFile, unsigned int(&dim)[3]) {


	short test[3];
	for (int i = 0; i < 3; i++) {
	
		
		inFile.read((char*)&test[i], sizeof(short));

	}
	dim[0] = int(test[0]);
	dim[1] = int(test[1]);
	dim[2] = int(test[2]);
}
//get voxel size of PET/CT image
void getVoxelSize(ifstream &inFile, float(&voxelSize)[3]) {
	for (int i = 0; i < 3; i++) {
		inFile.read((char*)&voxelSize[i], sizeof(float));
	}

}

ImageType::Pointer readVoiFilePET(string prjPath, string voiPath, ImageType *image, ConfigFile config, unsigned int(&dimPET)[3], float voxelSize[3]) {
	ImageType::Pointer maskImage;
	//parameters to store the dimension of the PET and CT image
	//the voxel size, halfLife of the tracer, the volume scale and the frame scale
	int nrVoxelsPET = int(dimPET[0] * dimPET[1] * dimPET[2]);
	//prjfile.close();
	//open the voi file
	ifstream voiFile;
	voiFile.open(voiPath, ios::in | ios::binary);
	if (voiFile.is_open() == false) {
		std::cout << "Cannot open voi file\n";
		exit(0);
	}
	else {
		//read the voi file
		streampos begin, end;
		begin = voiFile.tellg();
		voiFile.seekg(0, ios::end);
		end = voiFile.tellg();
		int fileSize = (end - begin);
		//vector to store all the elements
		vector<char> wholeFile(fileSize);

		voiFile.seekg(0, std::ios::beg);
		voiFile.read(&wholeFile[0], fileSize);
		vector< float> mask(fileSize);

		transform(wholeFile.begin(), wholeFile.end(), mask.begin(), [](auto& elem) {return ((float)(elem)); });

		vector< float> voi(wholeFile.begin() + (fileSize / 2), wholeFile.end());

		float *voiArray = &voi[0];
		//convert the array to an ITK image
		maskImage = converArray2Image(voiArray, dimPET, voxelSize);
		maskImage->Update();
		voiFile.close();
		//clean memory
		voiArray = nullptr;
		vector<float>().swap(voi);
		vector<float>().swap(mask);

	}
	return maskImage;
}