#include <string>
#include <vector>

#include "point.hpp"
#include "tinyxml.h"
#include "display.hpp"

using std::string;
using std::vector;



int xmlParser(string path){
    TiXmlDocument doc;

    // Load the XML file into the Doc instance
    doc.LoadFile(path.c_str());
    // Get root Element
    TiXmlElement* pWorld = doc.RootElement();
    // Print out the Table Header

    if (NULL != pWorld) {
        //Get 'World' Child
        TiXmlElement * pCamera = pWorld -> FirstChildElement("camera");

        if (NULL != pCamera) {
            // Get 'position' Child
            TiXmlElement * pPosition = pCamera -> FirstChildElement("position");
            if (NULL != pPosition) {
                string pos = pPosition -> GetText();
            }
            //Get 'lookAt' Child
            TiXmlElement * pLookAt = pCamera -> FirstChildElement("lookAt");
            if (NULL != pLookAt) {
                string lookat =  pLookAt -> GetText();
            }
            // Get 'up' Child
            TiXmlElement * pUp = pCamera -> FirstChildElement("up");
            if (NULL != pUp) {
                string up = pUp -> GetText();
            }
            // Get 'projection' Child
            TiXmlElement * pProjection = pCamera -> FirstChildElement("projection");
            if (NULL != pProjection) {
                string prj = pProjection -> GetText();

            }
        }

        TiXmlElement * pGroup = pWorld -> FirstChildElement("group");
        if (NULL != pCamera) {
            TiXmlElement * pModels = pGroup -> FirstChildElement("models");

            if (NULL != pModels) {
                TiXmlElement * pModel = pModels -> FirstChildElement("models");
                while(pModel){
                    string model = pModel -> GetText();

                    pModel = pModel -> NextSiblingElement("model");
                }
            }
        }
    }
}

int main(int argc, char **argv) {

    start(argv, argc);



    return 0;
}


