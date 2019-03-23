import sys
import os
import shutil

ExtensionName="HonCrack"
def cleanPackageDirectory(packageDir):
	if os.path.isdir(packageDir):
		print "Cleaning the existing package directory"
		shutil.rmtree(packageDir)
	else:
		print "No need to clean the package directory"

def layoutPackageDirectory(packageDir):
	global ExtensionName
	try:
		os.mkdir(packageDir)
	except OSError:
		print "Package Directory Already Exists..."
	try:
		print "Creating package directory structure."
		extensionDir = os.path.join(packageDir, ExtensionName)
		imageDir  = os.path.join(extensionDir, "images")
		binaryDir = os.path.join(extensionDir, "bin")
		xmlDir = os.path.join(extensionDir, "xml")
		htmlDir = os.path.join(extensionDir, "html")
		os.mkdir(extensionDir)
		os.mkdir(imageDir)
		os.mkdir(binaryDir)
		os.mkdir(xmlDir)
		os.mkdir(htmlDir)


	except OSError:
		print "Failed to create the Extension, Image or Binary directories"

def copyBinaryFiles(productsDir, packageDir):
	global ExtensionName
	print "Copying binary files..."
	extensionDir = os.path.join(packageDir, ExtensionName)
	binaryDir = os.path.join(extensionDir, "bin")
	# Just copy all of the contents of the productsDir over
	# to the bin dir
	for f in os.listdir(productsDir):
		if os.path.isfile(os.path.join(productsDir, f)):
			if os.path.splitext(f)[1]==".dll":
				shutil.copyfile(os.path.join(productsDir, f), os.path.join(binaryDir, f))
				print "Copying binary file: " + f


def copyImageFiles(productsDir, packageDir):
	global ExtensionName
	print "Copying image files..."
	extensionDir = os.path.join(packageDir, ExtensionName)
	imageDir  = os.path.join(extensionDir, "images")
	# Get to the actual images products dir
	imageSourceDir = os.path.abspath(os.path.join(productsDir, "..","..","images"))
	for f in os.listdir(imageSourceDir):
		if os.path.isfile(os.path.join(imageSourceDir, f)):
			if os.path.splitext(f)[1]==".bmp":
				shutil.copyfile(os.path.join(imageSourceDir, f), os.path.join(imageDir, f))
				print "Copying image file: " + f

def copyXMLFiles(productsDir, packageDir, BuildType):
	global ExtensionName
	print "Copying XML files..."
	extensionDir = packageDir
	xmlSourceDir = os.path.abspath(os.path.join(productsDir, "..","..","XML"))
	internalExtensionDir = os.path.join(packageDir, ExtensionName)
	xmlDir = os.path.join(internalExtensionDir, "xml")
	for f in os.listdir(xmlSourceDir):
		if os.path.isfile(os.path.join(xmlSourceDir, f)):
			if f=="HonCrack.xml":
				if BuildType=="Release":
					releaseBuildOfExtensionXML(os.path.join(xmlSourceDir, f), os.path.join(extensionDir, f))
				else:
					shutil.copyfile(os.path.join(xmlSourceDir, f), os.path.join(extensionDir, f))
				print "Copying XML file: " + f
			if f=="HonCrackDefaults.xml":
				shutil.copy(os.path.join(xmlSourceDir, f), os.path.join(xmlDir, f))
				print "Copying XML file: " + f

def copyHTMLFiles(productsDir, packageDir):
	global ExtensionName
	print "Copying HTML files..."
	extensionDir = packageDir
	htmlSourceDir = os.path.abspath(os.path.join(productsDir, "..","..","HTML"))
	internalExtensionDir = os.path.join(packageDir, ExtensionName)
	htmlDir = os.path.join(internalExtensionDir, "html")
	for f in os.listdir(htmlSourceDir):
		if os.path.isfile(os.path.join(htmlSourceDir, f)):
			shutil.copyfile(os.path.join(htmlSourceDir, f), os.path.join(htmlDir, f))
			print "Copying HTML file: " + f


def releaseBuildOfExtensionXML(xmlPath, destination):
	print "Augmenting the release XML build"
	with open(xmlPath,'r') as source:
		data=source.read().replace("<!-- INSERT ASSEMBLY HERE -->", "<assembly src=\"bin/HonCrack.dll\" namespace=\"PADT\"/>")
		with open(destination,'w') as target:
			target.write(data)

def debugBuildOfMainPython(mainPyPath, debugAssemblyPath, destination):
	print "Creating the debug build of the main.py file"
	with open(mainPyPath, 'r') as source:
		data=source.read().replace("#ASSEMBLY_PATH","assemblyPath = " + "\"" + debugAssemblyPath.replace("\\","\\\\")+"\"")
		with open(destination,'w') as target:
			target.write(data)

def releaseBuildOfMainPython(mainPyPath, destination):
	print "Creating the release build of the main.py file"
	with open(mainPyPath, 'r') as source:
		data=source.read().replace("#ASSEMBLY_PATH","assemblyPath = System.IO.Path.Combine(ExtAPI.ExtensionManager.CurrentExtension.InstallDir, \"bin\", \"HonyCrack.dll\")")
		with open(destination,'w') as target:
			target.write(data)

def copyPythonFiles(productsDir, packageDir):
	global ExtensionName
	print "Copying Python files..."
	extensionDir = os.path.join(packageDir, ExtensionName)
	pythonSourceDir = os.path.abspath(os.path.join(productsDir, "..","..","Python"))
	for f in os.listdir(pythonSourceDir):
		if os.path.isfile(os.path.join(pythonSourceDir, f)):
			if f=="main.py":
				shutil.copyfile(os.path.join(pythonSourceDir, f), os.path.join(extensionDir, f))
				print "Copying Python file: " + f

def copyPythonFiles(productsDir, packageDir, BuildType):
	global ExtensionName
	print "Copying Python files..."
	extensionDir = os.path.join(packageDir, ExtensionName)
	pythonSourceDir = os.path.abspath(os.path.join(productsDir, "..","..","Python"))
	for f in os.listdir(pythonSourceDir):
		if os.path.isfile(os.path.join(pythonSourceDir, f)):
			if f=="main.py":
				source = os.path.join(pythonSourceDir, f)
				destination = os.path.join(extensionDir, f)
				if BuildType=="Release":
					print "Building release build of main.py"
					releaseBuildOfMainPython(source, destination)
				else:
					print "Building debug build of main.py"
					debugAssembly = os.path.join(productsDir,"HonCrack.dll")
					debugBuildOfMainPython(source, debugAssembly, destination)

def copyDocumentation(productsDir, packageDir):
	global ExtensionName
	print "Copying Documentation files..."
	extensionDir = os.path.join(packageDir, ExtensionName)
	docsSourceDir = os.path.abspath(os.path.join(productsDir,"..","..","..","..","documentation","mechuser","site"))
	docsDestDir = os.path.join(extensionDir, "docs")
	shutil.copytree(docsSourceDir, docsDestDir)


def process(productsDir, packageDir, BuildType):
	print "Laying out the extension directory structure on disk..."
	cleanPackageDirectory(packageDir)
	layoutPackageDirectory(packageDir)
	copyBinaryFiles(productsDir, packageDir)
	copyImageFiles(productsDir, packageDir)
	copyXMLFiles(productsDir, packageDir, BuildType)
	copyPythonFiles(productsDir, packageDir, BuildType)
#	copyHTMLFiles(productsDir, packageDir)
#	copyDocumentation(productsDir, packageDir)

if len(sys.argv) == 4:
	productsDir=sys.argv[1]
	packageDir=sys.argv[2]
	buildType=sys.argv[3]
	process(productsDir, packageDir,buildType)
else:
	print "Could not acquire the directories from the command line."


