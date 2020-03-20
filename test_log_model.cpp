#include "RadarModel.hpp"


#include <iostream>
#include <vector>
#include <boost/random.hpp>
#include <boost/nondet_random.hpp>






const int NARGS = 8;

// quick build:
// Create build folder and go there
// cmake .. 
// make
//


/*
ffmpeg -r 20 -f image2 -s 1920x1080 -start_number 009 -i T3_S%03d_tempMap.png -vcodec libx264 -crf 25  -pix_fmt yuv420p test3.mp4

./mcdm/Images/mfc_test.pgm
120 p x 200 p (gimp) == 200 p x 120 p (ric) 

*/

int main(int argc, char **argv)
{

  boost::random::random_device                  rand_dev;
  boost::random::mt19937                        generator(rand_dev());


  std::string mapFileURI = "/home/pulver/mcdm/Images/mfc_test.pgm";
  double nx = 10;
  double ny = 10;
  double resolution = 0.1;
  double sigma_power = 4;
  double sigma_phase = 0.2;
  double txtPower = -10;
  int i = 0;
  char updateMode = 'b'; //w:white rec, p:pdf, b:bayesian
  cout <<"You provided: (" << argc-1 << ") arguments"<<  std::endl;

  if (argc > NARGS) {
            mapFileURI = argv[++i];
            nx = atoi(argv[++i]);
            ny = atoi(argv[++i]);
            resolution = atof(argv[++i]);
            sigma_power = atof(argv[++i]);
            sigma_phase = atof(argv[++i]);
            txtPower = atof(argv[++i]);
            updateMode = *argv[++i];
  } else {
      cout <<"DEFAULTS! I will execute call equivalent to:" << std::endl;
      cout << argv[0] << " " << mapFileURI  << " " << nx  << " " << ny  << " " << 
              resolution  << " " << sigma_power  << " " << sigma_phase  << " " << txtPower << " " << updateMode << std::endl;
  }
  cout <<"PARAMETERS: " << std::endl;
  // Radio model  ........................................................
  cout <<"\tReference Map file: ["<< mapFileURI <<"]" << std::endl;
  cout <<"\tRadar Active area: ("<< nx <<","<< ny <<") m." << std::endl;
  cout <<"\tMap Resolution: ("<< resolution <<") m./cell" << std::endl;
  cout <<"\tPower Noise std: ("<< sigma_power <<")" << std::endl;
  cout <<"\tPhase Noise std: ("<< sigma_phase <<")" << std::endl;
  cout <<"\tTransmitted power: ("<< txtPower <<") dB." << std::endl;
  
  cout <<"\tMap Update Mode: (";
  if (updateMode=='b'){
    cout <<"bayesian";
  } else if (updateMode=='p'){
    cout <<"pdf";
  } else if (updateMode=='P'){
    cout <<"pdf-noWalls";
  } else if (updateMode=='w'){
    cout <<"white rec.";
  } else if (updateMode=='G'){
    cout <<"No active areas used";
  }   
  cout <<  ") " << std::endl;
  
  cout << std::endl;
  
  
  // loading tag poses ........................................................
  // Unit is meters. We multiply pixels by resolution to get them.
  // 0,0 is at upper left of the map, with X increasing down and Y increasing right.

  //green
  double absTag1_X = 37 * resolution;
  double absTag1_Y = 58 * resolution;

  //blue
  double absTag2_X = 44 * resolution;
  double absTag2_Y = 31 * resolution;
  
  // red
  double absTag3_X = 130 * resolution;
  double absTag3_Y = 45 * resolution;
  
  // yellow
  double absTag4_X = 150 * resolution;
  double absTag4_Y = 108 * resolution;

  std::vector<std::pair<double,double>> tags_coord;
  tags_coord.push_back(std::make_pair(absTag1_X, absTag1_Y));
  tags_coord.push_back(std::make_pair(absTag2_X, absTag2_Y));
  tags_coord.push_back(std::make_pair(absTag3_X, absTag3_Y));
  tags_coord.push_back(std::make_pair(absTag4_X, absTag4_Y));
  //  ........................................................
  double f_i =  MIN_FREQ_NA; // 902e6 Hertzs

  // create frequencies vector from possible ones:
  
  /*
  Model with ALL freqs takes 1 sec to start. Too long for short tests
  std::vector<double> freqs;
  do{
    freqs.push_back(f_i);
    f_i+=STEP_FREQ_NA;
  } while (f_i<=MAX_FREQ_NA);
  */

  // std::vector<double> freqs{ MIN_FREQ_NA,MIN_FREQ_NA+STEP_FREQ_NA,MIN_FREQ_NA+2.0*STEP_FREQ_NA }; 
  std::vector<double> freqs{ 800e6,920e6  }; 

  // later on, we will use this to get random freqs...
  boost::random::uniform_int_distribution<>  distr(0, freqs.size()-1);

  //  ........................................................


  cout <<"Building radar model." << endl;
  RadarModel rm(nx, ny, resolution, sigma_power, sigma_phase, txtPower, freqs, tags_coord, mapFileURI );
  cout << "Radar model built." << endl;

  // prints reference map with tags
  rm.PrintRefMapWithTags("/tmp/test/scenario.png");      
  //  - Map goes from 0,0 to (N,M)*resolution in meters
  //  - Rcoods: up-left is 0,0. X increases down, and Y increases Right: swap x and y axes from images as they show on gimp 

  //rm.PrintRecPower("/tmp/test/rec_power_f_800.png", 800e6);
  //rm.PrintRecPower("/tmp/test/rec_power_f_920.png", 920e6);

  //rm.PrintPowProb("/tmp/test/prob_rec_power_95_f_800.png", -95, 800e6);
  //rm.PrintPowProb("/tmp/test/prob_rec_power_95_f_920.png", -95, 920e6);

  //rm.PrintPhase("/tmp/test/phase_f_800.png", 800e6);
  //rm.PrintPhase("/tmp/test/phase_f_920.png", 920e6);

  //rm.PrintPhaseProb("/tmp/test/prob_phase_45_f_800.png", 45.0*M_PI/180.0, 800e6);
  //rm.PrintPhaseProb("/tmp/test/prob_phase_45_f_920.png", 45.0*M_PI/180.0, 920e6);

  //rm.PrintBothProb("/tmp/test/prob_95db_45deg_f_800.png", -95, 45.0*M_PI/180.0, 800e6);
  //rm.PrintBothProb("/tmp/test/prob_95db_45deg_f_920.png", -95, 45.0*M_PI/180.0, 920e6);
  
  // simple scenario
  int NumReadings = 50;
  // Unit is meters. We multiply pixels by resolution to get them.
  double start_x = 40 * resolution;
  double start_y = 55 * resolution;
  double end_x = 150 * resolution;
  double end_y = 75 * resolution;
  
  double robot_y;
  double robot_x;
  double robot_head0=0;// atan2( end_y - start_y, end_x - start_x);
  double robot_head;
  double rxPower ;
  double tag_x, delta_x;
  double tag_y, delta_y;
  double phase;

  for (int t = 0; t < tags_coord.size(); t++){
    rm.saveProbMapDebug("/tmp/test/",t,0,start_x,start_y, robot_head0);
  }
  // let's do 10 samples from start point and ing a straight line.
  for (int i=0;i<NumReadings;i++){
      // current robot position, map coordinates.
      robot_y = start_y + ( (end_y - start_y) * ( i ) / (NumReadings - 1.0) );
      robot_x = start_x + ( (end_x - start_x) * ( i ) / (NumReadings - 1.0) );
      
      for (int h=0;h<36;h++){
        robot_head = robot_head0 + (2.0*M_PI*h/9);
        // std::cout<<"Robot at (" << robot_x << ", " << robot_y <<") m., (" << (robot_head*180.0/M_PI) << ") deg." << std::endl;
        
        //for each tag:
        for (int t = 0; t < tags_coord.size(); t++){
            // get relative robot-tag pose
            // translate
            delta_x = (tags_coord[t].first - robot_x ); 
            delta_y = (tags_coord[t].second - robot_y);
            // rotate
            tag_x =  delta_x * cos(robot_head) + delta_y * sin(robot_head);
            tag_y = -delta_x * sin(robot_head) + delta_y * cos(robot_head);

            // std::cout<<"\t- Tag [" << t << "] at position (" << tags_coord[t].first << ", " << tags_coord[t].second << ") m. " <<std::endl;
            // std::cout<<"\t\t  rel position (" << tag_x << ", " << tag_y << ") m. " <<std::endl;

            // get expected tag power with friis
            f_i = freqs[distr(generator)]; 
            rxPower = rm.received_power_friis( tag_x, tag_y, f_i, txtPower);

            // get expected phase from tag
            phase = rm.phaseDifference( tag_x,  tag_y,  f_i);
            //std::cout<<"\tReading at freq (" << f_i/1e6<< " MHz): (" << (rxPower+30) << ") dBm. ( " << phase << ") rads. " << std::endl << std::endl;

            // add measurement to model: addMeasurement0 draws a white rect, addMeasurement1 overlays the pdf, addMeasurement2 overlays the pdf but omits walls
            if (updateMode=='b'){
              rm.addMeasurement(robot_x, robot_y, robot_head*180.0/M_PI,  rxPower,  phase,  f_i,  t, false);
            } else if (updateMode=='p'){
              rm.addMeasurement1(robot_x, robot_y, robot_head*180.0/M_PI,  rxPower,  phase,  f_i,  t);
            } else if (updateMode=='P'){
              rm.addMeasurement2(robot_x, robot_y, robot_head*180.0/M_PI,  rxPower,  phase,  f_i,  t);
            } else if (updateMode=='w'){
              rm.addMeasurement0(robot_x, robot_y, robot_head*180.0/M_PI,  rxPower,  phase,  f_i,  t);
            } else if (updateMode=='G'){
              rm.addMeasurement3(robot_x, robot_y, robot_head*180.0/M_PI,  rxPower,  phase,  f_i,  t);
            }            
            

            //print maps
            //cout  << "Saving tag distribution maps... "<< endl;
            int lineal_index = (8*(i+1))+(h+1);
            rm.saveProbMapDebug("/tmp/test/",t,lineal_index,robot_x,robot_y, robot_head);
        }
        //std::cout<<"Finished reading. " << std::endl << std::endl;
    }
  }
  //print maps
  //cout  << "Saving tag distribution maps... "<< endl;
  rm.saveProbMaps("/tmp/test/");
  // for each tag:
  for (int t = 0; t < tags_coord.size(); t++){
    // cout << "---[" << t <<"]----------------" << endl;
   rm.saveProbMapDebug("/tmp/",t,0,0,0,0);
  }


  // lets play with the weights
  double w_i, w_max;
  double prec = 3;
  std::cout << std::fixed;
  std::cout << std::setprecision(2);

  std::cout << "Tag probability of being in " << (prec) << " square meters around different poses: " << std::endl;
  std::cout << "(Which happen to be tag reference poses...) " << std::endl;

  std::string sep = "\t\t";
    std::cout << "Tag \\ Pos" << "\t";
    for (int t = 0; t < tags_coord.size(); t++){
        tag_x = tags_coord[t].first;
        tag_y = tags_coord[t].second;
        std::cout << tag_x << ", " << tag_y << "\t";
  }
  std::cout << std::endl;

  for (int i = 0; i < tags_coord.size(); i++){
    cout  << " " << (i+1) << sep;
    w_max = rm.getTotalWeight(i);
    for (int t = 0; t < tags_coord.size(); t++){
        tag_x = tags_coord[t].first;
        tag_y = tags_coord[t].second;

        //w_i = rm.getTotalWeight(tag_x, tag_y, 0, i); // this uses the whole "active area" (i.e. 24x12 meters ) ... I don't think is really useful ...
        w_i = rm.getTotalWeight(tag_x, tag_y,0, sqrt(prec),sqrt(prec), i);
        cout  << (100.0 * w_i/w_max) << sep;
    }
    std::cout << std::endl;
  }


  return 0;
}



/*

  //rm.getImage("P_910.00", "/tmp/Average_power_map_f_910.png");            
  //rm.getImage("D_910.00", "/tmp/Average_phase_map_f_910.png");  
  //rm.PrintPowProb("/tmp/prob_rec_power_f_910.png", test_rxp, f_i);          
  //rm.PrintPhaseProb("/tmp/prob_rec_phase_f_910.png", test_phase, f_i);          
  
  
  // export tag locations into image, overimpressing background map.
  // at least points should be in in white areas1=1 




To read an arbitrary long tag loc list

  double absTag_pose_x;
  std::string absTag_string_x;
  double absTag_pose_y;
  std::string absTag_string_y;

  // Create an output string stream
  std::ostringstream streamObj3;

  // Set Fixed -Point Notation
  streamObj3 << std::fixed;

  // Set precision to 2 digits
  streamObj3 << std::setprecision(2);

  std::vector<std::pair<double,double>> tags_coord;
  for (int i=1;i<=numTags){
    // flush streamObj3
    streamObj3.clear();
    //streamObj3.str("");
    streamObj3 << numTags;

    absTag_string_x = streamObj3.str();
    absTag_string_y = streamObj3.str();
    absTag_pose_x = config[absTag_string_x].as<double>();
    absTag_pose_y = config[absTag_string_y].as<double>();
    tags_coord.push_back(std::make_pair(absTag_pose_x, absTag_pose_y));
  }

*/