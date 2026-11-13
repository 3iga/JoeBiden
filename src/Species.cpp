#include "Species.hpp"

// Biological descriptions are paraphrases; source URLs are also shown inside the field guide.
// All masses, swimming speeds and steering parameters below are illustrative game values.
const std::array<Species,SpeciesCount> SpeciesList={{
    {"Ocellaris clownfish","Amphiprion ocellaris",
     "A small orange reef fish with three pale bars. It lives in a close association with host sea anemones.",
     "A breeding group is led by a female; a breeding male can become female when she disappears.",
     "https://www.floridamuseum.ufl.edu/discover-fish/species-profiles/clown-anemonefish/",
     "clownfish.bmp",11,.025f,.70f,{.48f,.25f,.14f},{1,.39f,.08f,1},Behaviour::Hover},
    {"Blue tang","Paracanthurus hepatus",
     "A blue Indo-Pacific surgeonfish with a dark body marking and a yellow tail. It uses coral reefs as habitat.",
     "Like other surgeonfishes, it has a sharp spine on each side near the base of the tail.",
     "https://www.georgiaaquarium.org/animal/palette-surgeonfish/",
     "blue_tang.bmp",25.4f,.65f,1.7f,{.49f,.30f,.10f},{.06f,.30f,.97f,1},Behaviour::Cruise},
    {"Copperband butterflyfish","Chelmon rostratus",
     "A laterally flattened butterflyfish marked by copper-colored vertical bands and a long narrow snout.",
     "Its elongated snout helps it reach small food items in narrow reef crevices.",
     "https://www.georgiaaquarium.org/animal/copperband-butterflyfish/",
     "butterflyfish.bmp",20,.16f,.85f,{.40f,.34f,.09f},{1,.91f,.70f,1},Behaviour::Graze},
    {"Green humphead parrotfish","Bolbometopon muricatum",
     "A large parrotfish with a prominent forehead and beak-like teeth. It feeds on reef surfaces, including coral.",
     "Its feeding breaks down calcium-carbonate reef material and contributes to the production of sand.",
     "https://australian.museum/learn/animals/fishes/bumphead-parrotfish-bolbometopon-muricatum-valenciennes-1840/",
     "parrotfish.bmp",130,18,1.05f,{.58f,.30f,.22f},{.22f,.70f,.53f,1},Behaviour::Heavy},
    {"Yellow tang","Zebrasoma flavescens",
     "A bright yellow Pacific surgeonfish with a tall dorsal fin. It grazes on algae in coral-rich habitats.",
     "At night its coloration becomes darker and a pale lateral stripe becomes visible.",
     "https://www.georgiaaquarium.org/animal/yellow-tang/",
     "yellow_tang.bmp",20,.20f,1.25f,{.40f,.38f,.10f},{1,.86f,.04f,1},Behaviour::Graze},
    {"Moorish idol","Zanclus cornutus",
     "A reef fish with contrasting black, white and yellow bands and a long dorsal-fin streamer.",
     "Its striking silhouette comes from an extended dorsal-fin filament and a small projecting mouth.",
     "https://australian.museum/learn/animals/fishes/moorish-idol-zanclus-cornutus-linnaeus-1758/",
     "moorish_idol.bmp",24,.22f,1.0f,{.38f,.36f,.09f},{.99f,.91f,.68f,1},Behaviour::Cruise},
    {"Emperor angelfish","Pomacanthus imperator",
     "An Indo-Pacific angelfish. Adults have alternating blue and yellow stripes and a dark band through the eyes.",
     "Juveniles look very different, with pale curved lines and rings over a dark blue body.",
     "https://australian.museum/learn/animals/fishes/emperor-angelfish-pomacanthus-imperator/",
     "emperor_angelfish.bmp",38,1.1f,1.05f,{.52f,.34f,.12f},{.13f,.40f,.87f,1},Behaviour::Cruise},
    {"Blue-green chromis","Chromis viridis",
     "A small blue-green damselfish often found in groups above branching coral in sheltered reef habitats.",
     "It forms schools over corals; unlike the similar blackaxil puller, it lacks a black pectoral-fin base spot.",
     "https://lifg.australian.museum/Group.html?groupId=bWdTTXD5&hierarchyId=CEJQQmVx",
     "chromis.bmp",8,.018f,1.4f,{.40f,.24f,.105f},{.26f,.90f,.85f,1},Behaviour::Shoal}
}};
const char* behaviourName(Behaviour b) {
    switch(b) {
    case Behaviour::Hover:return "Home-range hovering";
    case Behaviour::Graze:return "Low reef foraging";
    case Behaviour::Shoal:return "Loose shoal steering";
    case Behaviour::Heavy:return "Slow wide turns";
    default:return "Cruising / wandering";
    }
}
