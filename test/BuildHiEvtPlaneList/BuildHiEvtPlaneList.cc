//
//  Program to create the HiEvtPlaneList.h header file starting with a .csv file that defines
//  the different event planes.
//
#include <stdio.h>
#include <iostream>
#include <math.h>
#include "string.h"

using namespace std;

char *parseNext(char *buf, int &indx, int &num);
void WriteEnumBlock(string vname, string *names, int num);
void WriteStringBlock(string vname, string *val, int num);
void WriteIntRefBlock(string vname, string *val, int num);
void WriteIntBlock(string vname, int *val, int num);
void WriteDoubleBlock(string vname, double *val, int num);
int Index[200];
string Name[200];
string Detector[200];
int Order[200];
double EtaMin1[200];
double EtaMax1[200];
double EtaMin2[200];
double EtaMax2[200];
double minTransverse[200];
double maxTransverse[200];
string ResCalcType[200];
string MomConsWeight[200];
string RescorMate1[200];
string RescorMate2[200];

char res[40];
FILE *fout;

int main() {
  FILE *fin = fopen("HiEvtPlaneList.csv", "r");
  fout = fopen("HiEvtPlaneList.h", "w");
  char buf[100];
  fgets(buf, 100, fin);
  fgets(buf, 100, fin);
  fgets(buf, 100, fin);
  fprintf(fout, "#ifndef __HiEvtPlaneList__\n");
  fprintf(fout, "#define __HiEvtPlaneList__\n");
  fprintf(fout, "/*\n");
  fprintf(fout, "Index     Name   Detector Order hmin1 hmax1 hmin2 hmax2 minpt maxpt nsub mcw    rmate1    rmate2\n");
  int nindx = 0;
  while (fgets(buf, 100, fin) != NULL) {
    int indx = 0;
    int num = 0;
    cout << buf << endl;
    int idx = atoi(parseNext(buf, indx, num));
    cout << "idx: " << idx << endl;
    if (idx == -10)
      break;
    Index[idx] = idx;
    Name[idx] = parseNext(buf, indx, num);
    Detector[idx] = parseNext(buf, indx, num);
    Order[idx] = atoi(parseNext(buf, indx, num));
    EtaMin1[idx] = atof(parseNext(buf, indx, num));
    EtaMax1[idx] = atof(parseNext(buf, indx, num));
    EtaMin2[idx] = atof(parseNext(buf, indx, num));
    EtaMax2[idx] = atof(parseNext(buf, indx, num));
    minTransverse[idx] = atof(parseNext(buf, indx, num));
    maxTransverse[idx] = atof(parseNext(buf, indx, num));
    ResCalcType[idx] = parseNext(buf, indx, num);
    MomConsWeight[idx] = parseNext(buf, indx, num);
    RescorMate1[idx] = parseNext(buf, indx, num);
    cout << "return" << endl;
    RescorMate2[idx] = parseNext(buf, indx, num);
    cout << "return" << endl;
    nindx = idx;
    fprintf(fout,
            "%5d%10s%10s%6d%6.2f%6.2f%6.2f%6.2f%6.2f%6.2f%5s%4s%10s%10s\n",
            idx,
            Name[idx].data(),
            Detector[idx].data(),
            Order[idx],
            EtaMin1[idx],
            EtaMax1[idx],
            EtaMin2[idx],
            EtaMax2[idx],
            minTransverse[idx],
            maxTransverse[idx],
            ResCalcType[idx].data(),
            MomConsWeight[idx].data(),
            RescorMate1[idx].data(),
            RescorMate2[idx].data());
  }
  ++nindx;
  fprintf(fout, "*/\n");
  fprintf(fout, "#include <string>\n");
  fprintf(fout, "using namespace std;\n");
  fprintf(fout, "namespace hi{\n");
  WriteEnumBlock("EPNamesInd", Name, nindx);
  WriteStringBlock("EPNames", Name, nindx);
  fprintf(fout, "\n  enum Detectors {Tracker, HF, Castor, RPD};\n");
  WriteIntRefBlock("EPDet", Detector, nindx);
  WriteIntBlock("EPOrder", Order, nindx);
  WriteDoubleBlock("EPEtaMin1", EtaMin1, nindx);
  WriteDoubleBlock("EPEtaMax1", EtaMax1, nindx);
  WriteDoubleBlock("EPEtaMin2", EtaMin2, nindx);
  WriteDoubleBlock("EPEtaMax2", EtaMax2, nindx);
  WriteDoubleBlock("minTransverse", minTransverse, nindx);
  WriteDoubleBlock("maxTransverse", maxTransverse, nindx);
  WriteStringBlock("ResCalcType", ResCalcType, nindx);
  WriteStringBlock("MomConsWeight", MomConsWeight, nindx);
  WriteIntRefBlock("RCMate1", RescorMate1, nindx);
  WriteIntRefBlock("RCMate2", RescorMate2, nindx);
  fprintf(fout, "\n  static const int NumEPNames = %d;\n", nindx);
  fprintf(fout, "}\n");
  fprintf(fout, "#endif\n");
}

char *parseNext(char *buf, int &indx, int &num) {
  num = 0;
  memset(res, 0, sizeof(res));
  strncpy(res, "-10", 3);
  if (buf[indx + num] == ',')
    return res;

  while (buf[indx + num] != ',') {
    if (buf[indx + num] == 0)
      return 0;
    ++num;
  }
  if (num == 0)
    return 0;
  memset(res, 0, sizeof(res));
  for (int i = 0; i < num; i++)
    res[i] = buf[indx + i];
  indx += num + 1;
  return &res[0];
}

void WriteEnumBlock(string vname, string *names, int num) {
  fprintf(fout, "\n  enum %s {\n  ", vname.data());
  for (int i = 0; i < num; i++) {
    fprintf(fout, "%12s,", names[i].data());
    if (fmod(i + 1, 5) == 0)
      fprintf(fout, "\n  ");
  }
  fprintf(fout, "   EPBLANK\n  };\n");
};

void WriteDoubleBlock(string vname, double *val, int num) {
  fprintf(fout, "\n  const double  %s[]  = {\n  ", vname.data());
  for (int i = 0; i < num - 1; i++) {
    fprintf(fout, "%12.2f,", val[i]);
    if (fmod(i + 1, 5) == 0)
      fprintf(fout, "\n  ");
  }
  fprintf(fout, "%12.2f", val[num - 1]);
  if (fmod(num, 5) == 0)
    fprintf(fout, "\n  ");
  fprintf(fout, " \n  };\n");
};

void WriteIntBlock(string vname, int *val, int num) {
  fprintf(fout, "\n  const int  %s[]  = {\n  ", vname.data());
  for (int i = 0; i < num - 1; i++) {
    fprintf(fout, "%12d,", val[i]);
    if (fmod(i + 1, 5) == 0)
      fprintf(fout, "\n  ");
  }
  fprintf(fout, "%12d", val[num - 1]);
  if (fmod(num, 5) == 0)
    fprintf(fout, "\n  ");
  fprintf(fout, " \n  };\n");
};

void WriteStringBlock(string vname, string *val, int num) {
  fprintf(fout, "\n  const std::string  %s[]  = {\n  ", vname.data());
  for (int i = 0; i < num - 1; i++) {
    string tmp = "\"";
    tmp += val[i];
    tmp += "\"";
    fprintf(fout, "%12s,", tmp.data());
    if (fmod(i + 1, 5) == 0)
      fprintf(fout, "\n  ");
  }
  string tmp = "\"";
  tmp += val[num - 1];
  tmp += "\"";
  fprintf(fout, "%12s", tmp.data());
  if (fmod(num, 5) == 0)
    fprintf(fout, "\n  ");
  fprintf(fout, " \n  };\n");
};

void WriteIntRefBlock(string vname, string *val, int num) {
  fprintf(fout, "\n  const int  %s[]  = {\n  ", vname.data());
  for (int i = 0; i < num - 1; i++) {
    fprintf(fout, "%10s,", val[i].data());
    if (fmod(i + 1, 5) == 0)
      fprintf(fout, "\n  ");
  }
  fprintf(fout, "%10s", val[num - 1].data());
  if (fmod(num, 5) == 0)
    fprintf(fout, "\n  ");
  fprintf(fout, " \n  };\n");
};
