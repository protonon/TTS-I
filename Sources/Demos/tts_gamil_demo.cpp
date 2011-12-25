#include "Sphere.h"
#include "TimeManager.h"
#include "GAMILReader.h"
#include "TTS.h"

int main(int argc, char **argv)
{
    TimeManager timeManager;
    GAMILReader gamilReader;
    TracerManager tracerManager;
    TTS tts;

    char fileName[30], filePattern[50] = "gamil_2562_%5.5d.nc";

    // -------------------------------------------------------------------------
    Sphere::setRadius(6371.299e3);

    // -------------------------------------------------------------------------
    tracerManager.init(argv[1]);

    // -------------------------------------------------------------------------
    gamilReader.init("gamil_data", "tts.gamil.suv.*.nc");
    gamilReader.getVelocityField();

    // -------------------------------------------------------------------------
    while (!timeManager.isFinished()) {
        tts.advect(gamilReader.meshManager, gamilReader.meshAdaptor,
                   gamilReader.flowManager, tracerManager);
        timeManager.advance();
        gamilReader.getVelocityField();
//        sprintf(fileName, filePattern, timeManager.getSteps());
//        tracerManager.output(fileName);
    }
}
