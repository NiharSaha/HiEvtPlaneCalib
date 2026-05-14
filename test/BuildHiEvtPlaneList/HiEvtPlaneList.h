#ifndef __HiEvtPlaneList__
#define __HiEvtPlaneList__
/*
Index     Name   Detector Order hmin1 hmax1 hmin2 hmax2 minpt maxpt nsub mcw    rmate1    rmate2
    0      HFm2        HF     2 -5.00 -3.00  0.00  0.00  0.01 30.00 3sub  no      HFp2 trackmid2
    1      HFp2        HF     2  3.00  5.00  0.00  0.00  0.01 30.00 3sub  no      HFm2 trackmid2
    2 trackmid2   Tracker     2 -0.50  0.50  0.00  0.00  0.50  3.00 3sub  no      HFm2      HFp2
    3   trackm2   Tracker     2 -0.50  0.00  0.00  0.00  0.50  3.00 3sub  no      HFp2      HFm2
    4   trackp2   Tracker     2  0.00  0.50  0.00  0.00  0.50  3.00 3sub  no      HFp2      HFm2
    5      HFm3        HF     3 -5.00 -3.00  0.00  0.00  0.01 30.00 3sub  no      HFp3 trackmid3
    6      HFp3        HF     3  3.00  5.00  0.00  0.00  0.01 30.00 3sub  no      HFm3 trackmid3
    7 trackmid3   Tracker     3 -0.50  0.50  0.00  0.00  0.50  3.00 3sub  no      HFm3      HFp3
    8   trackm3   Tracker     3 -0.50  0.00  0.00  0.00  0.50  3.00 3sub  no      HFp3      HFm3
    9   trackp3   Tracker     3  0.00  0.50  0.00  0.00  0.50  3.00 3sub  no      HFp3      HFm3
*/
#include <string>
using namespace std;
namespace hi {

  enum EPNamesInd { HFm2, HFp2, trackmid2, trackm2, trackp2, HFm3, HFp3, trackmid3, trackm3, trackp3, EPBLANK };

  const std::string EPNames[] = {
      "HFm2", "HFp2", "trackmid2", "trackm2", "trackp2", "HFm3", "HFp3", "trackmid3", "trackm3", "trackp3"

  };

  enum Detectors { Tracker, HF, Castor, RPD };

  const int EPDet[] = {HF, HF, Tracker, Tracker, Tracker, HF, HF, Tracker, Tracker, Tracker

  };

  const int EPOrder[] = {2, 2, 2, 2, 2, 3, 3, 3, 3, 3

  };

  const double EPEtaMin1[] = {-5.00, 3.00, -0.50, -0.50, 0.00, -5.00, 3.00, -0.50, -0.50, 0.00

  };

  const double EPEtaMax1[] = {-3.00, 5.00, 0.50, 0.00, 0.50, -3.00, 5.00, 0.50, 0.00, 0.50

  };

  const double EPEtaMin2[] = {0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00

  };

  const double EPEtaMax2[] = {0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00

  };

  const double minTransverse[] = {0.01, 0.01, 0.50, 0.50, 0.50, 0.01, 0.01, 0.50, 0.50, 0.50

  };

  const double maxTransverse[] = {30.00, 30.00, 3.00, 3.00, 3.00, 30.00, 30.00, 3.00, 3.00, 3.00

  };

  const std::string ResCalcType[] = {"3sub", "3sub", "3sub", "3sub", "3sub", "3sub", "3sub", "3sub", "3sub", "3sub"

  };

  const std::string MomConsWeight[] = {"no", "no", "no", "no", "no", "no", "no", "no", "no", "no"

  };

  const int RCMate1[] = {HFp2, HFm2, HFm2, HFp2, HFp2, HFp3, HFm3, HFm3, HFp3, HFp3

  };

  const int RCMate2[] = {trackmid2, trackmid2, HFp2, HFm2, HFm2, trackmid3, trackmid3, HFp3, HFm3, HFm3

  };

  static const int NumEPNames = 10;
}  // namespace hi
#endif
