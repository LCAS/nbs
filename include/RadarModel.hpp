#ifndef RADARMODEL_H
#define RADARMODEL_H


#include <grid_map_core/GridMap.hpp>
#include "grid_map_core/iterators/GridMapIterator.hpp"
#include "grid_map_cv/grid_map_cv.hpp"

#include "Eigen/Eigen"  // AFTER GRIDMAP!
#include <Eigen/Core>
#include <unsupported/Eigen/Splines>

// Math
#include <math.h>
#include <cmath>
#include <iostream>
#include <vector>

// other
#include <iomanip>
#include <string>

// OpenCV
#include <cv_bridge/cv_bridge.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "pose.h"
#include "data_struct.h"

using namespace std;
using namespace grid_map;

// constants ..................................................................
// We mostly use UPM frog 3D.
const double TAG_LOSSES = -4.8;

// 20 * log10 ( c / (4*pi) )
const double LOSS_CONSTANT = 147.55;
// 4*pi/c
const double PHASE_CONSTANT = 4.192e-8;
const double C = 299792458.0;

// This comes from the manufacturer. Azimut
// gain list entries start at -180 degrees to 180 in steps of 15.
const double ANTENNA_LOSSES_LIST [25] = {  -22.6, -25.2, -25, -20.2, -17.6, -15.6, -14, -11.2, -7.8, -5.2, -2.4, -0.6, 0, -0.6, -2.4, -5.2, -8.8, -12.2, -16.4, -19.2, -20.8, -24.4, -28.2, -24, -22.6};
const double ANTENNA_ANGLES_LIST [25] = {-180.0, -165.0, -150.0, -135.0, -120.0, -105.0, -90.0, -75.0, -60.0, -45.0, -30.0, -15.0, 0.0, 15.0, 30.0, 45.0, 60.0, 75.0, 90.0, 105.0, 120.0, 135.0, 150.0, 165.0, 180.0};


// M6e RFID reader Specs
// Minimum required power to identify a tag. Also depends on other factors, but
// this value is a guide
 const double SENSITIVITY = -115; // dB

// Max transmitted power may be limited by the Region regulations
// see Power Requirements  in M6e Hardware Guide
 const double MIN_TX_POWER = -25; // dB
 const double MAX_TX_POWER = 0; // dB
 const double STEP_TX_POWER = 0.5; // dB

// These freqs ARE limited depending on the region regulations
// see Regional Frequency Quantization in M6e Hardware Guide
const double STEP_FREQ = 25e3; // Hertzs
const double MIN_FREQ_A= 865e6; //Hertzs
const double MAX_FREQ_A= 869e6; //Hertzs
const double MIN_FREQ_B= 902e6; //Hertzs
const double MAX_FREQ_B= 928e6; //Hertzs

// most likely we will use EU or NA regions....
const double MIN_FREQ_EU= 865.6e6; //Hertzs
const double MAX_FREQ_EU= 867.6e6; //Hertzs
const double STEP_FREQ_EU = 100e3; // Hertzs

const double MIN_FREQ_NA= 902e6; //Hertzs
const double MAX_FREQ_NA= 928e6; //Hertzs
const double STEP_FREQ_NA = 250e3; // Hertzs

//  ..................................................................

class SplineFunction {
      public:
        SplineFunction();
        SplineFunction(Eigen::VectorXd const &x_vec, Eigen::VectorXd const &y_vec);

        double interpRad(double x) const;
        double interpDeg(double x) const;

      private:        
        double scaled_value(double x) const;

        Eigen::RowVectorXd scaled_values(Eigen::VectorXd const &x_vec) const;

        double x_min;
        double x_max;
        double y_min;
        double y_max;

        // Spline of one-dimensional "points."
        Eigen::Spline<double, 1> spline_;
};

//////////////////////////


class RadarModel
  {
    double _sizeXaa; //  x axis size in active area (m.). Related with number of rows
    double _sizeYaa; //  y axis size in active area (m.). Related with number of cols

    int _Ncol; // number of rows of reference and rfid belief maps (cells)
    int _Nrow; // number of cols of reference and rfid belief maps (cells)
    float _free_space_val; // max value stored in reference map, used as free space marker
    double _resolution; // Active areas, reference, and belief maps resolution (m./cell)
    double _txtPower; // transmitted power by RFID reader (dB)

    // we only model here gaussian noise: flat fadding.
    double _sigma_power;  // noise factor
    double _sigma_phase;  // noise factor
    
    GridMap _active_area_maps;  // active areas. one per phase and power at each frequency.
    GridMap _rfid_belief_maps;  // Prob. beliefs. One layer per tag. Also, one layer with reference map, mostly for tag layout representation.
    //GridMap _ref_map;  
    GridMap _tmp_rfid_c_map;
    
    std::vector<std::pair<double,double>> _tags_coords; // tag locations in reference map coords (m.)
    int _numTags;  // rfid tags to consider
    
    std::vector<double> _freqs; // transmission frequencies (Hz.)
    SplineFunction _antenna_gains;  // model for antenna power gain depending on the angle (dB.)

  public:

    double received_power_friis_with_obstacles(double antenna_x, double antenna_y, double antenna_h,
                                                       double tag_x, double tag_y, double tag_h,
                                                       double freq, double txtPower, SplineFunction antennaGainsModel);

    double received_power_friis_with_obstacles(double antenna_x, double antenna_y, double antenna_h,
                                                       double tag_x, double tag_y, double tag_h,
                                                       double freq);                                                       
    /**
     * @brief Plots a power distribution
     * 
     * @param fileURI save file location
     * @param f_i frequency to consider in the propagation model
     */
    void PrintRecPower(std::string fileURI, double f_i);

    /**
     * @brief Plots a probability distribution, conditioned to a received power
     * 
     * @param fileURI save file location
     * @param rxPw received power
     * @param f_i frequency to consider in the propagation model
     */
    void PrintPowProb(std::string fileURI, double rxPw, double f_i);




    RadarModel();
    RadarModel(const double nx, const double ny,  const double resolution, const double sigma_power, const double sigma_phase, const double txtPower, const std::vector<double> freqs, const std::vector<std::pair<double,double>> tags_coords, const std::string imageFileURI, std::string model="gaussian" ) ;
    void PrintMap( std::string savePath);
    void initRefMap(const std::string imageURI);
    void getImage(std::string layerName, std::string fileURI);

    /**
     * @brief Get layer name corresponding to given frequency (Hz.)
     * 
     * @param freq_i 
     * @return std::string 
     */
    std::string getLayerName(double freq_i);
  /////////////////////


/**
 * returns a range vector FROM start TO stop (included) in step increments
 * @param  start min Value in vector
 * @param  stop  max Value in vector
 * @param  step  increment
 * @return       range vector
 */
std::vector<double> range(double start, double stop, double step);

/**
 * returns MT_242025NRHK antenna losses in a plane
 * @param  angleRad spheric coordinate within the plane (radians)
 * @return          dB losses
 */
double antennaPlaneLoss(double angleRad );

/**
 * my Sign function...
 * @param  x
 * @return   sign of x
 */
float sign(float x);

/**
 * Returns Spherical coordinates
 * @param x          cartesian coordinate x (m.)
 * @param y          cartesian coordinate y (m.)
 * @param r          spheric coordinate r (m.)
 * @param phi        relative azimut [0,2pi) (angle between XY projection and X) (radians)
 */
void getSphericCoords(double x, double y, double& r, double& phi);

/**
 * Received signal estimated phase difference with pi ambiguity
 * @param  tag_x       Tag x coord (m.) with respect to antenna
 * @param  tag_y       Tag y coord (m.) with respect to antenna
 * @param  freq        Transmission frequency (Hertzs)
 * @return      phase difference (radians)
 */
double phaseDifference(double tag_x, double tag_y, double freq);

Eigen::MatrixXf getFriisMat(double x_m, double y_m, double orientation_deg, double freq);
Eigen::MatrixXf getPhaseMat(double x_m, double y_m, double orientation_deg, double freq);
Eigen::MatrixXf getProbCond(Eigen::MatrixXf X_mat, double x, double sig);

/**
 * Get tag detection boundaries for given configuration
 * @param freq        RF signal frequency
 * @param txtPower    Transmitted RF power (dB.)
 * @param sensitivity Minimun power the reader can receive (dB.)
 * @param distStep    distance step for internal power calculations
 * @param minX        minimum X distance where rx power is over sensitivity
 * @param minY        minimum Y distance where rx power is over sensitivity
 * @param maxX        maximum X distance where rx power is over sensitivity
 * @param maxY        maximum Y distance where rx power is over sensitivity
 */
void activeAreaFriis(double freq, double txtPower, double sensitivity, double distStep, double& minX, double& minY, double& maxX, double& maxY);



/**
 * Calculate the sum of the values in a portion of the belief map
 * 
 * @param x: x-coord (m.) of the center
 * @param y: y-coord (m.) of the center
 * @param orientation: orientation (rad.) in map coords of the center
 * @param size_x: size-x of the active area 
 * @param size_y: size-y of the active area
 * @param tag_i: id of the tag emitting the signal
 */
double getTotalWeight(double x, double y, double orientation, double size_x, double size_y, int tag_i);
/**
 * Calculate the sum of the values in a portion of the belief map
 * 
 * @param x: x-coord (m.) of the center
 * @param y: y-coord (m.) of the center
 * @param orientation: orientation (rad.) in map coords of the center
 * @param iterator: iterator over a submap 
 * @param tag_i: id of the tag emitting the signal
 */
double getTotalWeight(double x, double y, double orientation, grid_map::SubmapIterator  iterator, int tag_i);
double getTotalWeight(double x, double y, double orientation, int tag_i);
double getTotalWeight(int tag_i);

/**
 * Calculate the entropy of the tag position over the map
 * 
 * @param x: x-coord (m.) of the center
 * @param y: y-coord (m.) of the center
 * @param orientation: orientation (rad.) in map coords of the center
 * @param size_x: size-x of the active area 
 * @param size_y: size-y of the active area
 * @param tag_i: id of the tag emitting the signal
 */
double getTotalEntropy(double x, double y, double orientation,  double size_x, double size_y, int tag_i);
/**
 * Calculate the entropy of the tag position over the map
 * 
 * @param x: x-coord (m.) of the center
 * @param y: y-coord (m.) of the center
 * @param orientation: orientation (rad.) in map coords of the center
 * @param iterator: iterator over a submap 
 * @param tag_i: id of the tag emitting the signal
 */
double getTotalEntropy(double x, double y, double orientation, grid_map::SubmapIterator  iterator, int tag_i);

/**
 * Calculate the KL-divergence between posterior and prior distribution over tags position
 * 
 * @param x: x-coord (m.) of the center
 * @param y: y-coord (m.) of the center
 * @param orientation: orientation (rad.) in map coords of the center
 * @param size_x: size-x of the active area 
 * @param size_y: size-y of the active area
 * @param tag_i: id of the tag emitting the signal
 */
double getTotalKL(double x, double y, double orientation,  double size_x, double size_y, int tag_i);
/**
 * Calculate the KL-divergence between posterior and prior distribution over tags position
 * 
 * @param x: x-coord (m.) of the center
 * @param y: y-coord (m.) of the center
 * @param orientation: orientation (rad.) in map coords of the center
 * @param iterator: iterator over a submap 
 * @param tag_i: id of the tag emitting the signal
 */
double getTotalKL(double x, double y, double orientation, grid_map::SubmapIterator  iterator, int tag_i);

void addMeasurement(double x, double y, double orientation, double rxPower, double phase, double freq, int i);
void addMeasurement0(double x, double y, double orientation, double rxPower, double phase, double freq, int i);
void addMeasurement1(double x, double y, double orientation, double rxPower, double phase, double freq, int i);
void addMeasurement2(double x, double y, double orientation, double rxPower, double phase, double freq, int i);
void addMeasurement3(double x_m, double y_m, double orientation_deg, double rxPower, double phase, double freq, int i);


void addTmpMeasurementRFIDCriterion(double x, double y, double orientation, double rxPower, double phase, double freq, int i, double len_update);

std::string getPowLayerName(double freq_i);

std::string getPhaseLayerName(double freq_i);

std::string getTagLayerName(int tag_num);


/**
 * Get received power from an OMNIDIRECTIONAL tag,
 * given its relative position to antenna.
 * We assume antenna at 0,0,0, facing X coordinate.
 * See http://www.antenna-theory.com/basics/friis.php
 * Sensitivity is -85 dBm / -115 dB
 *
 * @param  tag_r       Tag r coord (m.) with respect to antenna
 * @param  tag_h       Tag h coord (rad.) with respect to antenna
 * @param  freq        Transmission frequency (Hertzs)
 * @param  txtPower    Transmitted power (dB)
 * @param antennaGainsModel   Antena Gain model
 * @return             Received power (dB)
 */
double received_power_friis_polar(double tag_r, double tag_h, double freq, double txtPower, SplineFunction antennaGainsModel);

void getImageDebug(GridMap* gm, std::string layerName, std::string fileURI);
Eigen::MatrixXf getPowProbCond(double rxPw, double f_i);
Eigen::MatrixXf  getPhaseProbCond(double ph_i, double f_i);
Eigen::MatrixXf  getProbCond(std::string layer_i, double x, double sig);

Eigen::MatrixXf getPowProbCondRFIDCriterion(double rxPw, double f_i);
Eigen::MatrixXf getIntervProbRFIDCriterion( double x, double sigm, double len_update);
Eigen::MatrixXf getNegProbRFIDCriterion( double sensitivity, double sigm);

void saveProbMapDebug(std::string savePATH, int tag_num, int step, double robot_x, double robot_y, double robot_head);
void createTempProbLayer(Eigen::MatrixXf prob_mat, double x_m, double y_m, double orientation_deg);
void createTempProbLayerRFIDCriterion(Eigen::MatrixXf prob_mat, double x_m, double y_m, double orientation_deg, double len_update);
cv::Mat rfidBeliefToCVImg(std::string layer_i);
void getImage(GridMap* gm,std::string layerName, std::string fileURI);
Position getRelPoint(Position glob_point, double x_m, double  y_m, double orientation_rad);
Position getSubMapRelPoint(Position glob_point, double x_m, double  y_m, double orientation_rad, double len);
Eigen::MatrixXf getIntervProb(std::string layer_i, double x, double sigm);
void fillFriisMat(Eigen::MatrixXf *rxPw_mat, Eigen::MatrixXf *delay_mat, double freq_i, double offset );
void PrintRefMapWithTags(std::string fileURI);
void PrintRecPower(std::string fileURI, double f_i);
void PrintPowProb(std::string fileURI, double rxPw, double f_i);
void PrintPhase(std::string fileURI,  double f_i);
void PrintPhaseProb(std::string fileURI, double phi, double f_i);
void PrintBothProb(std::string fileURI, double rxPw, double phi, double f_i);
void overlayRobotPose(double robot_x, double robot_y, double robot_head, cv::Mat& image);
void overlayRobotPoseT(double robot_x, double robot_y, double robot_head, cv::Mat& image);
void rotatePoints( cv::Point* points, int npts, int cxi, int cyi, double ang);
void clearObstacles(cv::Mat& image);

Eigen::MatrixXf  getProbCondG(std::string layer_i, double x, double sig);
Eigen::MatrixXf  getProbCondLogN(std::string layer_i, double x, double sig);

void PrintProb(std::string fileURI, Eigen::MatrixXf* prob_mat,  double sX, double sY, double res);
void saveProbMaps(std::string savePath);

grid_map::Position fromPoint(cv::Point cvp);
grid_map::Polygon getActiveMapEdges(double robot_x, double robot_y, double robot_head);
grid_map::Polygon getSubMapEdges(double robot_x, double robot_y, double robot_head, double len);

void PrintProb(std::string fileURI, Eigen::MatrixXf* prob_mat);

std::pair<int, std::pair<int, int>>findTagFromBeliefMap(int num_tag);

void normalizeRFIDLayer(std::string layerName);
void normalizeRFIDMap();
void clearObstacleCellsRFIDMap();
double getNormalizingFactorBayesRFIDActiveArea(double x_m, double y_m, double orientation_rad, string tagLayerName);
double getNormalizingFactorBayesFullMap(double x_m, double y_m, double orientation_rad, string tagLayerName);
Eigen::MatrixXf getNegProb(std::string layer_i, double sensitivity, double sigm);

void debugInfo();


cv::Point getPoint(double x, double y);
void overlayActiveMapEdges(double robot_x, double robot_y, double robot_head, cv::Mat image);
void overlayMapEdges( cv::Mat image);

/**
 * Return the private active area
 */
GridMap getActiveAreaMaps();

/**
 * Return the RFID belief maps
 */
GridMap getBeliefMaps();

/**
 * Return the value used for identifying free cells
 */
float getFreeSpaceVal();

void cutPowerBasedObstacleDistribution(double* rxPower, Pose* target, std::pair<int, int> relTagCoord);

}; // end class

#endif