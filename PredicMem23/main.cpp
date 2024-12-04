/////////////////////////////////////////////////////////////////////////////////
//                                                                             //
//    Copyright (c) 2024  Pablo Sánchez Cuevas                    //
//                                                                             //
//    This file is part of PredicMem23.                                            //
//                                                                             //
//    PredicMem23 is free software: you can redistribute it and/or modify          //
//    it under the terms of the GNU General Public License as published by     //
//    the Free Software Foundation, either version 3 of the License, or        //
//    (at your option) any later version.                                      //
//                                                                             //
//    PredicMem23 is distributed in the hope that it will be useful,               //
//    but WITHOUT ANY WARRANTY; without even the implied warranty of           //
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the              //
//    GNU General Public License for more details.                             //
//                                                                             //
//    You should have received a copy of the GNU General Public License        //
//    along with PredicMem23. If not, see <
// http://www.gnu.org/licenses/>.
//
//                                                                             //
/////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include "Experimentation.h"

int main(int argc, char* argv[])
{
    if (argc > 2) {
        string msg = "ERROR: Only the path to the input XML file must be given!\n";
        std::cout << msg;
        throw std::invalid_argument(msg);
    }
    else if (argc == 1) {
        string msg = "ERROR: The path to the input XML file must be given!\n";
        std::cout << msg;
        throw std::invalid_argument(msg);
    }

    string inputSpecsFilePath = string(argv[1]);

    TracePredictExperimentation::performAndExportExperimentations(inputSpecsFilePath);

}

