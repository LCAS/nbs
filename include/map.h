#ifndef MAP_H
#define MAP_H

#include <fstream> // ifstream
#include <sstream> // stringstream
#include <vector>
#include <string>
#include <iostream>
#include "pose.h"
#include <unordered_map>
#include "RFIDGridmap.h"

using namespace std;
namespace dummy{

class Map
{

public:

  Map(std::ifstream& infile, double resolution, double imgresolution);	//constructor, takes in binary map file .pgm
	//Map(nav_msgs::OccupancyGrid ros_msg);
 // ~Map();					//destructor
  Map();
  void setGridValue(int value, long i, long j);	//setter for value in position ij of the grid
  int getGridValue(long i, long j) const;		//getter for value in position ij of the grid
  long getGridValue(long i) const;			//getter for value in position i of the grid vector
  long getMapValue(long i, long j);		//getter for value in position ij of the map
  long getNumGridRows() const;
  long getNumGridCols() const;
  long getNumRows();
  long getNumCols();
  void findFreeEdges(int cX, int cY);
  void addEdgePoint(int x, int y);
  //std::vector<int> getMap();
  //std::vector<int> getGrid();
  std::vector<vector<long> > getMap2D();
  std::vector<long> grid;			//vector containing the map as grid of cells sized 1 square metre
  std::vector<int> pathPlanningGrid;
  std::vector<int> RFIDGrid;
  long numGridRows;
  long numGridCols;
  int gridToPathGridScale;
  Pose getRobotPosition();
  long getTotalFreeCells();
  void setCurrentPose(Pose &p);
  void drawVisitedCells();
  void printVisitedCells(vector<string> &history);
  int getPathPlanningGridValue(long i,long j) const;
  void setPathPlanningGridValue(int value, int i, int j);
  int getPathPlanningNumCols() const;
  int getPathPlanningNumRows() const;
  int getGridToPathGridScale() const;
  void updatePathPlanningGrid(int x, int y, int rangeInMeters, double power);
  std::pair<int, int> getRelativeTagCoord(int absTagX, int absTagY, int antennaX, int antennaY);
  void setRFIDGridValue(float power, int i, int j);
  void drawRFIDScan();
  void drawRFIDGridScan(RFIDGridmap grid);
  int getRFIDGridValue(long i,long j) const;
  void updateRFIDGrid(double power, double phase, int antennaX, int antennaY);
  std::pair<int, int> findTag();
  std::pair<int, int> findTagfromGridMap(RFIDGridmap grid);
  std::pair<long, long> getRandomFreeCell();
	//nav_msgs::OccupancyGrid toROSMsg();
protected:
  std::vector<long> map;				//vector containing the original map as binary matrix (0 -> free, 1 -> obstacle)
  void createMap(std::ifstream& infile);
  void createGrid(double resolution);
  void createPathPlanningGrid(double resolution);
//  void createRFIDGrid(double resolution);
  void createNewMap();
  int numPathPlanningGridRows;
  int numPathPlanningGridCols;
  long numRows;
  long numCols;
  std::vector<std::pair<int, int> > edgePoints;
  long totalFreeCells;
  void decreaseFreeCells();
  Pose currentPose;
  std::vector<pair<long, long>> listFreeCells;

};
}

#endif
