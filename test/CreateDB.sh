#!/bin/bash
#======================================
#
# This script runs through the steps needed to create a new event plane
# calibration.  A new calibration is required whenever a change is made
# to the header file
#    RecoHI/HiEvtPlaneAlgos/interface/HiEvtPlaneList.h
# and whenever a change is made to one of the event plane parameters, such
# the pseudorapdity or pt rangle. (That is, ANY change to the default
# parameters found in the 
#    RecoHI/HiEvtPlaneAlgos/python/HiEvtPlane_cfi.py 
# or
#    RecoHI/HiEvtPlaneAlgos/python/hiEvtPlaneFlat_cfi.py
# file. (Note that these two python files MUST have common default values.)
#
# For most calibrations it will be necessary to handle Steps 1 and 4 by submitting crab jobs.
# Sample crab submission scripts are also found supplied in this directory:
# crabSubmit_calib   (for Step 1)
# crabSubmit_check   (for Step 4)

#=======================================

#=======================================
#
#  STEP 1.  Perform an initial replay of the dataset being calibrated.
#           This replay will create a root file called calib.root containing
#           the data needed for subsequent calibration steps.
#
#========================================
echo  ==================================================================
echo  ">>Step pre-0.  The code expects certain files to be in fixed locations"
echo  ">>             In this directory (the run directory) you need the following:"
echo  ">>                  calibtree_cfg.py    (Script to replay data files and produce a tree structure for subsequent analysis.)"
echo  ">>                  checkep_cfg.py      (Script to test that the final results.)"
echo  ">>                  moveflatparamstodb_cfg.py  (Script to create an sqlite database file with the flattening parameters.)"
echo  ">>                  The EPCalib directory (This is the location of the code that determines the flattening parameters using the previously create tree.)"
echo  ">>              In the EPCalib directory, in addition to the code already present, you need the following:"
echo  ">>                  A copy of $CMSSW_RELEASE_BASE/src/RecoHi/HiEvtPlaneAlgos/interface/HiEvtPlaneList.h"
echo  ">>                  A --modified-- copy of $CMSSW_RELEASE_BASE/src/RecoHI/HiEvtPlaneAlgos/interface/HiEvtPlaneFlatten.h"
echo  ">>                         (The modification requires commenting out the FWCore and DataFormats include statements.)"
echo  " "
echo  "start? (yes) [yes/no]":
read allgo
if [ $allgo = "no" ] ; then
	exit
fi
echo  ===================================================================
echo  ">>Step 0.  Several files are created in running this script.  These can"
echo  ">>         be quite large, depending on how many events are analyzed to"
echo  ">>         create the calibration.   By default, the files will be created"
echo  ">>         in your working directory (pwd):"
pwd
echo  ">>         If you would like to use a different location for these file,"
echo  ">>         enter it here:${NC}"
echo  ===================================================================
echo  "  "
str="Directory to store generated files (pwd):"
echo  -n $str
read defloc
if [ -z $defloc ]; then defloc="./"; fi
echo "Base directory set to: "$defloc
echo  ===================================================================
echo  ">>Input type.  Select whether to run on MC simulation or real 2024 PbPb data:"
echo  ">>         MC:    Hydjet MiniAODSIM (run=1, used for setup and validation)"
echo  ">>         Data:  2024 PbPb HIRun2024A MiniAOD (runs 387853-388784)"
echo  ==================================================================
str="Input type (MC/Data) [MC]:"
echo -n $str
read inputtype
if [ -z $inputtype ]; then inputtype="MC"; fi
if [ $inputtype != "MC" ] && [ $inputtype != "Data" ]; then
	echo "Only allowed options are MC or Data"
	exit
fi
echo "Input type set to: "$inputtype
echo  ===================================================================
echo  ">>Step 1.  Create the calib.root file with the data needed for the subsequent"
echo  ">>         calibration steps.  The input type selected above (MC or Data) determines"
echo  ">>         which files are read.  Replay type options:"
echo  ">>         "
echo  ">>         MiniAOD:     Read input files defined in calibtree_cfg.py."
echo  ">>                      MC:   Hydjet MinBias MiniAODSIM (HINPbPbWinter24, 5.36 TeV)"
echo  ">>                      Data: 2024 PbPb HIRun2024A HIPhysicsRawPrime0 MiniAOD PromptReco-v1"
echo  ">>         testMiniAOD: A single MiniAOD file selected by the MINITEST environment variable."
echo  ">>         skip:        Skip this step (reuse an existing calib.root)."
echo  ">>         "
echo  ">>         Since the replay accesses the"
echo  ">>         CMS database you need to first issue the command:"
echo  ">>         voms-proxy-init -voms cms"
echo  ">>         The code echos the parameters being assumed for the"
echo  ">>         replay.  Patience might be needed...  No additional input will be requested"
echo  ">>         by this script."
echo  ==================================================================
echo  "  "
str="Replay type (MiniAOD):"
echo -n $str
read aodtype
if [ -z $aodtype ]; then aodtype="MiniAOD"; fi
echo "Entered:"$aodtype
# checkaodtype is used for Step 4; when Step 1 is skipped, default to MiniAOD
if [ $aodtype = "skip" ]; then
	checkaodtype="MiniAOD"
	infile=''
else
	checkaodtype=$aodtype
fi
if [ $aodtype != "skip" ] ; then
	if [ $aodtype = "MiniAOD" ] ; then
		echo "MiniAOD type selected"
		infile=''
	elif [ $aodtype = "testMiniAOD" ]  ; then
    		echo "testMiniAOD type selected"
    		if [ -z $MINITEST ]; then
			echo "The global parameter MINITEST has not been set."
			echo "To do this from a BASH shell, issue the command:"
			echo "export MINITEST={MiniAOD file}"
			exit
    		else
			echo "Replay will be from the file: "$MINITEST
			infile=$MINITEST
    		fi
	else
    		echo "Only allowed options are MiniAOD|testMiniAOD|skip"
    		exit
	fi

	echo voms-proxy-init -voms cms
	voms-proxy-init -voms cms
	COM="cmsRun calibtree_cfg.py outfile="$defloc"/calib.root repFile="$infile" aodType="$aodtype" inputType="$inputtype""
	echo  $COM
	$COM
fi
echo  ===================================================================
echo  ">>Step 2.  Generate the event plane recentering/flattening parameters"
echo  ">>         This is done with a standalone program that takes the"
echo  ">>         calib.root file generated in Step 1 as input."
echo  ">> "
echo  ">>         Although, to keep this script simple, we only have one input file for"
echo  ">>         this step, in general one would expect many files with one file generated for"
echo  ">>         each crab job.  The list of files to be used in the calibration"
echo  ">>         is specified in temphi.lis .  This script will not handle the more general case."
echo  ">>         In that case, the script can be used to demonstrate the general framework."
echo  ">> "
echo  ==================================================================
echo  " "       

cd EPCalib
#[ -h HiEvtPlaneList.h ] && rm -f HiEvtPlaneList.h
#[ -f HiEvtPlaneList.h ] && rm -f HiEvtPlaneList.h
#ln -s $CMSSW_BASE/src/RecoHI/HiEvtPlaneAlgos/interface/HiEvtPlaneList.h 
#[ -h HiEvtPlaneFlatten.h ] && rm -f HiEvtPlaneFlatten.h
#[ -f HiEvtPlaneFlatten.h ] && rm -f HiEvtPlaneFlatten.h
#ln -s $CMSSW_BASE/src/RecoHI/HiEvtPlaneAlgos/interface/HiEvtPlaneFlatten.h
cd ..
[ -d RescorTables ] && rm -rf RescorTables
calfile=$defloc"/calib.root"
ls -1 $calfile > tmphi.lis
echo "Listing of tmphi.lis:"
cat tmphi.lis
declare -a list=(387853 388785)  # 2024 PbPb run range: 387853-388784
# calibrunmin/calibrunmax filter events from the calib.root tree (Step 2).
# They are SEPARATE from the DB IOV (which comes from 'list' above).
if [ $inputtype = "MC" ]; then
	calibrunmin=0        # MC events have run=1; accept all runs
	calibrunmax=999999
else
	calibrunmin=387853   # 2024 PbPb real data run range
	calibrunmax=388784
fi
nbreaks=${#list[@]}
echo $nbreaks
for (( i=0; i<${nbreaks}-1; i++));
do
min=${list[$i]}
max=${list[$i+1]}
max=$((max-1))
minrun=$min
maxrun=$max
range=$minrun
range+='_'
range+=$maxrun 
resdir=$defloc"/RescorTables"
[ ! -d $resdir ] && mkdir $resdir
resdir=$defloc"/RescorTables/Rescor_"$range
[ -d $resdir ] && rm -rf $resdir
mkdir $resdir
offfile=$defloc"/offset_"$range".root"
echo $offfile
[ -f $offfile ] && rm $offfile
tmpfile=$defloc"/tmp_"$range".root"
echo $tmpfile
[ -f $tmpfile ] && rm $tmpfile
rpflat=$defloc"/rpflat_"$range".root"
echo $rpflat
[ -f $rpflat ] && rm $rpflat
epfile=$defloc"/ep_"$range".root"
echo $epfile
[ -f $epfile ] && rm $epfile
arg='EPCalib/EPCalib.C+('$calibrunmin','$calibrunmax',"tmphi.lis","'$tmpfile'","'$epfile'","'$offfile'","'$resdir'")'
echo $arg
root -l -b -q $arg
# Sanity check: verify EPtree was written to the ep file
eptree_check=$(root -l -b -q -e "TFile *f=TFile::Open(\"$epfile\"); if(!f||f->IsZombie()){cout<<\"FAIL\"<<endl;exit(1);}TTree*t=(TTree*)f->Get(\"EPtree\");if(!t){cout<<\"FAIL\"<<endl;exit(1);}cout<<\"OK \"<<t->GetEntries()<<endl;" 2>/dev/null | grep -E "^OK|^FAIL")
if [[ "$eptree_check" != OK* ]]; then
    echo "ERROR: EPCalib.C did not produce a valid EPtree in $epfile"
    echo "       Possible cause: calibrunmin/calibrunmax ($calibrunmin/$calibrunmax) filtered out all events."
    echo "       For MC (run=1): use calibrunmin=0 calibrunmax=999999"
    echo "       Check that calib.root exists and contains events in the run range."
    exit 1
fi
echo ">> EPtree check passed: $eptree_check entries"
echo ">>The following files/directories should now have been created:"
echo $resdir'   -- Directory containing the event plane resolutions'
echo $rpflat'   -- File with results of the flattening operation'
echo $offfile'   --File with offsets that can be used in a track-by-track correction'
[ -f $tmpfile ] && rm $tmpfile

echo  ===================================================================
echo  " "
echo  "Step 3.  Move calibration to database file"
echo  " "
echo  ===================================================================

ln -s $epfile rpflat_combined.root
move='cmsRun moveflatparamstodb_cfg.py print outputFile=HeavyIonRPRcd_'$range'.db outtag=HeavyIonRPRcd begin='$minrun' end='$maxrun' rescorloc='$resdir' infile='$epfile
echo $move
$move
[ -f EPCalib/EPCalib_C.d ] && rm EPCalib/EPCalib_C.d
[ -f EPCalib/EPCalib_C.so ] && rm EPCalib/EPCalib_C.so
[ -f EPCalib/EPCalib_C_ACLiC_dict_rdict.pcm ] && rm EPCalib/EPCalib_C_ACLiC_dict_rdict.pcm
rm -rf rpflat_combined.root

conddb_import -f sqlite_file:HeavyIonRPRcd_$range.db -c sqlite_file:HeavyIonRPRcd_offline.db -i HeavyIonRPRcd -t HeavyIonRPRcd -b $minrun -e $maxrun
[ -f HeavyIonRPRcd_$range.db ] && rm HeavyIonRPRcd_$range.db
done
echo ===================================================================
echo ">> "
echo ">>The final calibration file, containing the calibrations for the different IOVs, is called"
echo ">>HeavyIonRPRcd_offline.db"
echo ">> "
echo ===================================================================
[ -f tmphi.lis ] && rm tmphi.lis

# Create a check DB with IOV starting at run 1 so MC events (Run=1) are covered.
cmsRun moveflatparamstodb_cfg.py print outputFile=HeavyIonRPRcd_check.db outtag=HeavyIonRPRcd begin=1 end=999999 rescorloc=$resdir infile=$epfile

echo  ===================================================================
echo  ">> "
echo  ">>Step 4.  Check that everything works.   The new calibration is used to"
echo  ">>         replay the raw data a second time, now with the flattening being done."
echo  ">>         A final root program generates several histograms compaing the input"
echo  ">>         data and final results.   The original (black) histograms should be"
echo  ">>         completed covered by the final (red) histograms.  ANY black showing would indicated"
echo  ">>         a potential problem, assuming the exact same events are processed. (This is often not true.)"
echo  ">> "
echo  ===================================================================
COM="cmsRun checkep_cfg.py checkfile="$defloc"/checkep.root repFile="$infile" aodType="$checkaodtype" dbfile=HeavyIonRPRcd_check.db inputType="$inputtype""
echo $COM
$COM
[ -f HeavyIonRPRcd_check.db ] && rm HeavyIonRPRcd_check.db
calibFile=$defloc"/calib.root"
moveFile=$epfile
checkFile=$defloc"/checkep.root"
root -l -b -q  'compare.C("'$calibFile'","'$moveFile'","'$checkFile'")'
#[ -f $epfile ] && rm $epfile

