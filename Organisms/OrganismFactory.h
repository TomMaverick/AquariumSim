#ifndef AQUARIUMSIM_ORGANISMFACTORY_H
#define AQUARIUMSIM_ORGANISMFACTORY_H

#include <memory>
#include "Animals/NeonTetra.h"
#include "Animals/NeocaridinaDavidi.h"
#include "Plants/Hornwort.h"
#include "Plants/AnubiasNana.h"
#include "Plants/HygrophilaPolyspermaRosanervig.h"
#include "Plants/RotalaOrangeJuice.h"
#include "Plants/MossBall.h"
#include "Plants/CryptocoryneWendtii.h"

class OrganismFactory {
public:
    static std::unique_ptr<Animal> createNeocaridina() { return std::make_unique<NeocaridinaDavidi>(); }
    static std::unique_ptr<Animal> createNeonTetra() { return std::make_unique<NeonTetra>(); }

    static std::unique_ptr<Plant> createHornwortStem() { return std::make_unique<Hornwort>(); }
    static std::unique_ptr<Plant> createAnubias() { return std::make_unique<AnubiasNana>(); }
    static std::unique_ptr<Plant> createHygrophila() { return std::make_unique<HygrophilaPolyspermaRosanervig>(); }
    static std::unique_ptr<Plant> createRotala() { return std::make_unique<RotalaOrangeJuice>(); }
    static std::unique_ptr<Plant> createMossBall() { return std::make_unique<MossBall>(); }
    static std::unique_ptr<Plant> createCryptocoryne() { return std::make_unique<CryptocoryneWendtii>(); }
};

#endif //AQUARIUMSIM_ORGANISMFACTORY_H