#include "RadarModel.hpp"

using namespace std;
using namespace grid_map;


        SplineFunction::SplineFunction()
        {}

        SplineFunction::SplineFunction(Eigen::VectorXd const &x_vec, Eigen::VectorXd const &y_vec)
          : x_min(x_vec.minCoeff()),
            x_max(x_vec.maxCoeff()),
            y_min(y_vec.minCoeff()),
            y_max(y_vec.maxCoeff()),
            // Spline fitting here. X values are scaled down to [0, 1] for this.
            spline_(Eigen::SplineFitting<Eigen::Spline<double, 1>>::Interpolate(y_vec.transpose(), std::min<int>(x_vec.rows() - 1, 6), scaled_values(x_vec)))  // No more than cubic spline, but accept short vectors.
        { 

            // for(int i=0; i< x_vec.size(); i++){
            //       cout <<"G(" << x_vec[i] << ")= "<< interpDeg(x_vec[i]) << std::endl;
            // } 

        }
        
        // x values need to be scaled down in extraction as well.
        double SplineFunction::interpDeg(double x) const {      
          double y;
          y = spline_(scaled_value(x))(0);


//          if ((y>y_max) || (y<y_min)){
//                  cout <<"Angle " << x << " produced gain "<< y << std::endl;
//          }

          // interpolation may produce values bigger and lower than our limits ...
          
          y = max(min(y, y_max), y_min );
          return y;
        }

        double SplineFunction::interpRad(double x) const {
            return interpDeg(x*180.0/M_PI); 
        }
      
        // Helpers to scale X values down to [0, 1]
        double SplineFunction::scaled_value(double x) const {
          return (x - x_min) / (x_max - x_min);
        }

        Eigen::RowVectorXd SplineFunction::scaled_values(Eigen::VectorXd const &x_vec) const {
          return x_vec.unaryExpr([this](double x) { return scaled_value(x); }).transpose();
        }

//////////////////




RadarModel::RadarModel(const double nx, const double ny, const double resolution, const double sigma_power, const double sigma_phase, const double txtPower, const std::vector<double> freqs, const std::vector<std::pair<double,double>> tags_coords, const std::string imageFileURI ) {
        _sizeX = nx;
        _sizeY = ny;
        _sigma_power = sigma_power;
        _sigma_phase = sigma_phase;
        _txtPower = txtPower;
        _freqs = freqs;
        _resolution = resolution;
        _tags_coords = tags_coords;
        _numTags =  tags_coords.size();

        initRefMap(imageFileURI);

        //cout <<"HI!" << std::endl;
        // build spline to interpolate antenna gains;
        std::vector<double> xVec(ANTENNA_ANGLES_LIST, ANTENNA_ANGLES_LIST + 25);
        Eigen::VectorXd xvals = Eigen::Map<Eigen::VectorXd, Eigen::Unaligned>(xVec.data(), xVec.size());
        std::vector<double> yVec(ANTENNA_LOSSES_LIST, ANTENNA_LOSSES_LIST + 25);
        Eigen::VectorXd yvals= Eigen::Map<Eigen::VectorXd, Eigen::Unaligned>(yVec.data(), yVec.size());
        
        //cout <<"HI2!" << std::endl;
        _antenna_gains= SplineFunction(xvals, yvals);
        //cout <<"antenna gain spline model built" << std::endl;


        // build internal grid
        // grid goes from -X/2,-Y/2 to X/2,Y/2 with resolution
        _map.setGeometry(Length(_sizeX,_sizeY), _resolution);
        _rfid_belief_maps.setGeometry(Length(_NIC,_NJC), _resolution);

        // This equals to a Size
        Size siz = _map.getSize();
      
        // create r, h matrixes
        // Eigen::MatrixXd r_mat = Eigen::MatrixXd(siz(0), siz(1));
        // Eigen::MatrixXd h_mat = Eigen::MatrixXd(siz(0), siz(1));

        // Create map of estimated received powers at reference each frequency 
        Position point;
        double tag_x, tag_y, tag_r, tag_h,phi, rxP;
        Index ind;
        std::string layerName;

        for(std::vector<double>::size_type i = 0; i != _freqs.size(); i++) {
              
              Eigen::MatrixXf rxPw_mat = Eigen::MatrixXf(siz(0), siz(1));
              Eigen::MatrixXf delay_mat = Eigen::MatrixXf(siz(0), siz(1));

              for (grid_map::GridMapIterator iterator(_map); !iterator.isPastEnd(); ++iterator) {
                  // matrix indexes...
                  ind = *iterator;
                  
                  // get cell center of the cell in the map frame.            
                  _map.getPosition(ind, point);
                  // that is where the tag is.
                  tag_x = point.x();
                  tag_y = point.y();

                  getSphericCoords(tag_x,tag_y, tag_r, tag_h);

                  // PHASE_CONSTANT  === 4*pi/c 
                  phi = fmod ( PHASE_CONSTANT * _freqs[i] * tag_r, M_PI);

                  rxP = received_power_friis_polar(tag_r, tag_h, _freqs[i], _txtPower, _antenna_gains);
                  //cout <<"rxPw: ("<< rxP <<")" << std::endl;
                  rxPw_mat(ind(0),ind(1))  = rxP;                  
                  delay_mat(ind(0),ind(1))  = phi;                  
              }

              // we have like an mean values map for each frequency and position with this power and setup ...
              layerName = getPowLayerName(freqs[i]);
              _map.add(layerName, rxPw_mat);
              layerName = getPhaseLayerName(freqs[i]);
              _map.add(layerName, delay_mat);
              cout <<"Models for F "<< getLayerName(freqs[i]) << " built" << std::endl;
        }

        // rfid belief global map: One layer per tag
        for(int i = 0; i <_numTags; ++i) {
           layerName = std::to_string(i);
           _rfid_belief_maps.add(layerName, 0.0);
        }



        debugInfo();




    }



void RadarModel::initRefMap(const std::string imageURI){

        // This is a hacky way to get a global area in the radar model equal to the image size and resolution
        _imageCV = cv::imread(imageURI , CV_LOAD_IMAGE_UNCHANGED );

        // CV: Rotate 90 Degrees COUNTER To get our data to have increasing X to right and increasing Y up
        cv::flip(_imageCV, _imageCV, 1);
        cv::transpose(_imageCV, _imageCV);
        
        _NIC = _imageCV.rows; // radar model total x-range space (cells).
        _NJC = _imageCV.cols;  // radar model total x-range space (cells).

        // cell value ranges
        double  minValue;
        double  maxValue;

        //2D position of the grid map in the grid map frame [m].
        double orig_x;
        double orig_y;

        // orig will be placed at the CENTER OF THE IMAGE
        //orig_x= 0;
        //orig_y= 0;

        _origMap= grid_map::GridMap (vector<string>({"layer"}));
        _origMap.setGeometry(Length(_NIC, _NJC), _resolution);
        //_origMap.setGeometry(Length(_NIC, _NJC), _resolution, Position(orig_x, orig_y));


        cv::minMaxLoc(_imageCV, &minValue, &maxValue);
        _origMap["layer"].setConstant(NAN);
        GridMapCvConverter::addLayerFromImage<unsigned char, 3>(_imageCV, "layer", _origMap, minValue, maxValue);
        
        grid_map::Position p;       
        grid_map::Index index;
        
        index =grid_map::Index(0,0);
        _origMap.getPosition(index,p);  
        std::cout<<"P: (" << p(0) << ", " << p(1)<<") m. == Cell("  << index(0) << ", " << index(1) << ")" <<std::endl;

        index =grid_map::Index(_NIC,0);
        _origMap.getPosition(index,p);  
        std::cout<<"P: (" << p(0) << ", " << p(1)<<") m. == Cell("  << index(0) << ", " << index(1) << ")" <<std::endl;

        index =grid_map::Index(0,_NJC);
        _origMap.getPosition(index,p);  
        std::cout<<"P: (" << p(0) << ", " << p(1)<<") m. == Cell("  << index(0) << ", " << index(1) << ")" <<std::endl;

        index =grid_map::Index(_NIC,_NJC);
        _origMap.getPosition(index,p);  
        std::cout<<"P: (" << p(0) << ", " << p(1)<<") m. == Cell("  << index(0) << ", " << index(1) << ")" <<std::endl;

        

}



    //So, robot at pr (x,y,orientation) (long, long, int) receives rxPower,phase,freq from tag i . 
    void RadarModel::addMeasurement(double x_m, double y_m, double orientation_deg, double rxPower, double phase, double freq, int i){
      // cout <<"Position: (" << x_m << " m., " << y_m << " m., " << orientation_deg <<"º) " << std::endl;
      // cout <<"Measurement: Tag ["<< i <<  "]: (" << rxPower << " dB, " << phase << " rad., " << freq/1e6 <<"MHz.) " << std::endl;
      // cout <<"........................ " << std::endl;

      // Probability distribution associated with ( rxPower,phase,freq) in a grid with resolution rx,  defined in  (-nx/2,nx/2) x (-ny/2,ny/2), is:        
      // Boundaries are relative to position (0,0,0º): p1 (nx/2,ny/2) , p2 (-nx/2, ny/2), p3 (-nx/2,-ny/2), p4 (nx/2,-ny/2): 
      Eigen::MatrixXf prob_mat = getPowProbCond(rxPower, freq).cwiseProduct(getPhaseProbCond(phase, freq));
      _map.add("temp", prob_mat);      

      // With that data, we can update the area around robot position and orientation pr defined by the rectangle : p1+pr, p2+pr, p3+pr, p4+pr 
      double rel_x, rel_y, prob_val, orientation_rad;
      orientation_rad = orientation_deg * M_PI/180.0;

      double glob_x, glob_y;
      Position point;

      std::string tagLayerName = std::to_string(i);
      

      for (grid_map::GridMapIterator iterator(_map); !iterator.isPastEnd(); ++iterator) {                  
                  // get cell center of the cell in the map frame.            
                  _map.getPosition(*iterator, point);                  
                  rel_x = point.x();
                  rel_y = point.y();

                  // cast position to global map
                  glob_x =  rel_x * cos(orientation_rad) + rel_y * sin(orientation_rad) + x_m;
                  glob_y = -rel_x * sin(orientation_rad) + rel_y * cos(orientation_rad) + y_m;

                  // cout <<"rel: (" << rel_x << ", " << rel_y << ") " << std::endl;
                  // cout <<"glob: (" << glob_x << ", " << glob_y << ") " << std::endl;
                  // cout <<"........................ " << std::endl;

                  point = Position(glob_x, glob_y);
                  // check if is inside global map
                  if (_rfid_belief_maps.isInside(point)){
                        // get the prob:
                        prob_val = _map.at("temp",*iterator);

                        // add value
                        _rfid_belief_maps.atPosition(tagLayerName,point)+= 10*prob_val;
                  }
                  

      }

      // python friendly debug:
      // cout <<"PR =[" << x_m << ", " << y_m << ", " << orientation_rad <<"] " << std::endl;
      // cout <<"rel_x =[] " << std::endl;
      // cout <<"rel_y =[] " << std::endl;
      // cout <<"glob_x =[] " << std::endl;
      // cout <<"glob_y =[] " << std::endl;
      // for (grid_map::GridMapIterator iterator(_map); !iterator.isPastEnd(); ++iterator) {                  
      //               // get cell center of the cell in the map frame.            
      //               _map.getPosition(*iterator, point);                  
      //               rel_x = point.x();
      //               rel_y = point.y();

      //               // cast position to global map
      //               glob_x =  rel_x * cos(orientation_rad) + rel_y * sin(orientation_rad) + x_m;
      //               glob_y = -rel_x * sin(orientation_rad) + rel_y * cos(orientation_rad) + y_m;

      //               cout <<"rel_x.append(" << rel_x << ") " << std::endl;
      //               cout <<"rel_y.append(" << rel_y << ") " << std::endl;
      //               cout <<"glob_x.append(" << glob_x << ") " << std::endl;
      //               cout <<"glob_y.append(" << glob_y << ") " << std::endl;
                    
      // }
      // cout <<"#.................................................. " << std::endl << std::endl;

    }

    void  RadarModel::saveProbMaps(std::string savePath){
        std::string tagLayerName;
        std::string fileURI;
        Eigen::MatrixXf data_mat ;

        PrintMap(savePath);


        // prob distribution maps
        for(int i = 0; i <_numTags; ++i) {
          tagLayerName = std::to_string(i);
          fileURI  = savePath + "final_prob_" +tagLayerName + ".png";
          data_mat = _rfid_belief_maps[tagLayerName];
          PrintProb(fileURI, &data_mat, _NIC*_resolution, _NJC*_resolution, _resolution);
        }
    }

    //print original map with tag locations
    void RadarModel::PrintMap( std::string savePath){
        std::string fileURI;
        cv::Mat image;
        grid_map::Index index;            
        cv::Scalar green( 0, 255, 0 );
        cv::Scalar blue( 255, 0, 0 );      
        cv::Scalar red( 0, 0, 255 );
        cv::Scalar yellow( 0, 255, 255 );      


        fileURI  = savePath + "ref_map.png";


        
        // Convert to image.

        const float minValue = _origMap["layer"].minCoeff();
        const float maxValue = _origMap["layer"].maxCoeff();

        GridMapCvConverter::toImage<unsigned char, 3>(_origMap, "layer", CV_8UC3, minValue, maxValue, image);
      
        //add tag locations into cv mat:              
        cv::Point center;

        for (int i = 0; i < _tags_coords.size(); i++){
            double x = _tags_coords[i].first;
            double y =  _tags_coords[i].second;
            grid_map::Position p(x,  y);                    
            _origMap.getIndex(p,index);         
            std::cout<<"Green: (" << p(0) << ", " << p(1)<<") m. == Cell("  << index(0) << ", " << index(1) << ")" <<std::endl;
                           
            center = cv::Point( index.x(), index.y() );
            //cv::circle( image,center,6,green,-1,8 );
            cv::circle(image, center , 10, green, -1);

        }
        // map references: ..............................................................
      // double maxX=  _origMap.getLength().x()/2;
      // double maxY=  _origMap.getLength().y()/2;

      // grid_map::Position p(0,  0);
      // _origMap.getIndex(p,index);      
      // cv::Point gree( index.y(), index.x() );
      // std::cout<<"Green: (" << p(0) << ", " << p(1)<<") m. == Cell("  << index(0) << ", " << index(1) << ")" <<std::endl;

      // p = Position(maxX,  -maxY);
      // _origMap.getIndex(p,index);      
      // cv::Point blu( index.y(), index.x() );
      // std::cout<< "Blue (" << p(0) << ", " << p(1)<<") m. == Cell("  << index(0) << ", " << index(1) << ")" <<std::endl;

      // p = Position(-maxX,  -maxY);
      // _origMap.getIndex(p,index);  
      // cv::Point re( index.y(), index.x() );
      // std::cout<<"Red (" << p(0) << ", " << p(1)<<") m. == Cell("  << index(0) << ", " << index(1) << ")" <<std::endl;

      // p = Position(-maxX,  maxY);
      // _origMap.getIndex(p,index);  
      // cv::Point yell( index.y(), index.x() );
      // std::cout<<"Yellow (" << p(0) << ", " << p(1)<<") m. == Cell("  << index(0) << ", " << index(1) << ")" <<std::endl;
      
      // cv::circle(image, gree , 20, green, -1);
      // cv::circle(image, blu , 20, blue, -1);
      // cv::circle(image, re ,  20, red, -1);
      // cv::circle(image, yell , 20, yellow, -1);







        // CV: Rotate 90 Degrees Clockwise To get our images to have increasing X to right and increasing Y up
        cv::transpose(image, image);
        cv::flip(image, image, 1);
        cv::imwrite( fileURI, image );
    
    }



    Eigen::MatrixXf  RadarModel::getPowProbCond(double rxPw, double f_i){
      std::string layerName = getPowLayerName(f_i);
      Eigen::MatrixXf ans = getProbCond(layerName, rxPw, _sigma_power);
      return ans;
    }

    Eigen::MatrixXf  RadarModel::getProbCond(std::string layer_i, double x, double sig){

        double W = 0;
        double den = 0;

        // create a copy of the average values
        Eigen::MatrixXf av_mat = _map[layer_i];
        
        // cout <<"Layer name: ("<< layer_i <<")" << std::endl;
        // cout <<"x value: ("<< x <<")" << std::endl;
        // cout <<"Layer av. val sumatory: ("<< av_mat.sum() <<")" << std::endl;
        // cout <<"Layer Min Pow: ("<< _map[layer_i].minCoeff() <<")" << std::endl;
        // cout <<"Layer Max Pow: ("<< _map[layer_i].maxCoeff()  <<")" << std::endl;


        // gaussian prob fddp
        den = sig *sqrt( 2.0 * M_PI) ;
        // cout <<"den: ("<< den <<")" << std::endl;

        av_mat = (x - av_mat.array())/sig;
        // cout <<"t1: ("<< av_mat.sum() <<")" << std::endl;

        av_mat =  av_mat.array().pow(2.0);
        // cout <<"t2: ("<< av_mat.sum() <<")" << std::endl;

        av_mat = -0.5 * av_mat;
        // cout <<"t3: ("<< av_mat.sum() <<")" << std::endl;

        av_mat = av_mat.array().exp()/den;
        // cout <<"t4: ("<< av_mat.sum() <<")" << std::endl;

        // normalize in this space:
        av_mat = av_mat/ (av_mat.sum());
        // cout <<"t5: ("<< av_mat.sum() <<")" << std::endl;

        return av_mat;
    }

    Eigen::MatrixXf  RadarModel::getPhaseProbCond(double ph_i, double f_i){
        std::string layerName = getPhaseLayerName(f_i);
        Eigen::MatrixXf ans = getProbCond(layerName, ph_i, _sigma_phase);
        return ans;
    }    

    void RadarModel::PrintPowProb(std::string fileURI, double rxPw, double f_i){
        Eigen::MatrixXf prob_mat = getPowProbCond(rxPw, f_i);
        PrintProb(fileURI, &prob_mat);
    }

    void RadarModel::PrintPhaseProb(std::string fileURI, double phi, double f_i){
        Eigen::MatrixXf prob_mat = getPhaseProbCond(phi, f_i);
        PrintProb(fileURI, &prob_mat);
    }

    void RadarModel::PrintBothProb(std::string fileURI, double rxPw, double phi, double f_i){
        Eigen::MatrixXf prob_mat = getPowProbCond(rxPw, f_i).cwiseProduct(getPhaseProbCond(phi, f_i));
        PrintProb(fileURI, &prob_mat);
    }

    void RadarModel::PrintProb(std::string fileURI, Eigen::MatrixXf* prob_mat){      
        PrintProb(fileURI, prob_mat, _sizeX, _sizeY, _resolution);
    }

    void RadarModel::PrintProb(std::string fileURI, Eigen::MatrixXf* prob_mat, double sX, double sY, double res){
        GridMap tempMap;      
        tempMap.setGeometry(Length(sX, sY), res);
        tempMap.add("res", *prob_mat);
        
        // float minValue = tempMap["res"].minCoeff();
        // float maxValue = tempMap["res"].maxCoeff();

        // cout <<"MinProb: ("<< minValue <<")" << std::endl;
        // cout <<"MaxProb: ("<< maxValue  <<")" << std::endl;

        getImage(&tempMap, "res", fileURI);
    }



  
    void RadarModel::debugInfo(){
        std::cout << ".................." << std::endl;
        Length mlen = _map.getLength();
        std::cout << "Active area is: "<< mlen(0) << " by " << mlen(1) << " m. (x,y)" <<std::endl;
        std::cout << "Resolution is: " << _resolution<< " m. /cell" << std::endl;
        std::cout << "Active grid is: "<< _map.getSize()(0) << " by " << _map.getSize()(1) << " cells (i,j axis)" <<std::endl;
        std::cout << ". " << std::endl;
        std::cout << "Total grid Map is: "<< _NIC << " by " << _NJC << " cells (i,j axis)" <<std::endl;

        // for (const auto& layer : _map.getLayers()) {
        //     std::cout << ((std::string) layer) << std::endl;
        // }
        std::cout << ".................." << std::endl;
    }



    void RadarModel::getImage(std::string layerName, std::string fileURI){
        getImage(&_map, layerName, fileURI);
    }

    void RadarModel::getImage(GridMap* gm,std::string layerName, std::string fileURI){

    
      // Convert to image.
      cv::Mat image;
      const float minValue = (*gm)[layerName].minCoeff();
      const float maxValue = (*gm)[layerName].maxCoeff();

      GridMapCvConverter::toImage<unsigned char, 3>(*gm, layerName, CV_8UC3, minValue, maxValue, image);
      

      // create a triangle pointing x+
      cv::Scalar green( 0, 255, 0 );
      cv::Scalar blue( 255, 0, 0 );      
      cv::Scalar red( 0, 0, 255 );
      cv::Scalar yellow( 0, 255, 255 );      
      grid_map::Index index;            
      cv::Point triang_points[1][3];
      double h=_map.getLength().x()/15.0;      
      grid_map::Position p(h/2.0,  0);
      gm->getIndex(p,index);        
      // !!! stupid opencv indexing !!!!
      triang_points[0][0] = cv::Point( index.y(), index.x() );
      p = Position(-h/2.0,  -h/2.0);
      gm->getIndex(p,index);  
      triang_points[0][1] = cv::Point( index.y(), index.x() );
      p = Position(-h/2.0,  h/2.0);
      gm->getIndex(p,index);  
      triang_points[0][2] = cv::Point( index.y(), index.x() );
      const cv::Point* ppt[1] = { triang_points[0] };
      int npt[] = { 3 };
      cv::fillPoly( image,ppt,npt,1,red,8 );

      // CV: Rotate 90 Degrees Clockwise To get our images to have increasing X to right and increasing Y up
      cv::transpose(image, image);
      cv::flip(image, image, 1);
      cv::imwrite( fileURI, image );

    }

    void RadarModel::getImageDebug(std::string layerName, std::string fileURI){

      // Convert to image.
      cv::Mat image;
      const float minValue = _map[layerName].minCoeff();
      const float maxValue = _map[layerName].maxCoeff();

      GridMapCvConverter::toImage<unsigned char, 3>(_map, layerName, CV_8UC3, minValue, maxValue, image);
      
      cv::Scalar green( 0, 255, 0 );
      cv::Scalar blue( 255, 0, 0 );      
      cv::Scalar red( 0, 0, 255 );
      cv::Scalar yellow( 0, 255, 255 );
      
      grid_map::Index index;
      

      double maxX=  _map.getLength().x()/2;
      double maxY=  _map.getLength().y()/2;

      grid_map::Position p(maxX,  maxY );
      _map.getIndex(p,index);      
      cv::Point gree( index.x(), index.x() );
      std::cout<<"Green: (" << p(0) << ", " << p(1)<<") m. == Cell("  << index(0) << ", " << index(1) << ")" <<std::endl;

      p = Position(maxX,  -maxY);
      _map.getIndex(p,index);      
      cv::Point blu( index.y(), index.x() );
      std::cout<< "Blue (" << p(0) << ", " << p(1)<<") m. == Cell("  << index(0) << ", " << index(1) << ")" <<std::endl;

      p = Position(-maxX,  -maxY);
      _map.getIndex(p,index);  
      cv::Point re( index.y(), index.x() );
      std::cout<<"Red (" << p(0) << ", " << p(1)<<") m. == Cell("  << index(0) << ", " << index(1) << ")" <<std::endl;

      p = Position(-maxX,  maxY);
      _map.getIndex(p,index);  
      cv::Point yell( index.y(), index.x() );
      std::cout<<"Yellow (" << p(0) << ", " << p(1)<<") m. == Cell("  << index(0) << ", " << index(1) << ")" <<std::endl;
      
      cv::circle(image, gree , 20, green, -1);
      cv::circle(image, blu , 20, blue, -1);
      cv::circle(image, re ,  20, red, -1);
      cv::circle(image, yell , 20, yellow, -1);
      
      cv::Point triang_points[1][3];
      double h=0.2;
      
      p = Position(h/2.0,  0);
      _map.getIndex(p,index);        
      triang_points[0][0] = cv::Point( index.y(), index.x() );
      std::cout<<"p (" << p(0) << ", " << p(1)<<") m. == Cell("  << index(0) << ", " << index(1) << ")" <<std::endl;

      p = Position(-h/2.0,  -h/2.0);
      _map.getIndex(p,index);  
      triang_points[0][1] = cv::Point( index.y(), index.x() );
      std::cout<<"p (" << p(0) << ", " << p(1)<<") m. == Cell("  << index(0) << ", " << index(1) << ")" <<std::endl;

      p = Position(-h/2.0,  h/2.0);
      _map.getIndex(p,index);  
      triang_points[0][2] = cv::Point( index.y(), index.x() );
      std::cout<<"p (" << p(0) << ", " << p(1)<<") m. == Cell("  << index(0) << ", " << index(1) << ")" <<std::endl;

      const cv::Point* ppt[1] = { triang_points[0] };
      int npt[] = { 3 };
      cv::fillPoly( image,ppt,npt,1,red,8 );

      
      // Rotate 90 Degrees Clockwise To get our images to have increasing X to right and increasing Y up
      cv::transpose(image, image);
      cv::flip(image, image, 1);
      cv::imwrite( fileURI, image );

    }

    /*
      cv::Point rook_points[1][3];

      const cv::Point* ppt[1] = { rook_points[0] };
      int npt[] = { 3 };
      //cv::fillPoly( image,ppt,npt,1,yellow,8 );
      // grid_map::Polygon polygon;
      // polygon.setFrameId(_map.getFrameId());
      // polygon.addVertex(Position( 0, h/2 ));
      // polygon.addVertex(Position(  -h/2,  h/2 ));
      // polygon.addVertex(Position( h/2, -h/2 ));
      // for (grid_map::PolygonIterator iterator(map_, polygon); !iterator.isPastEnd(); ++iterator) {
      //     map_.at("type", *iterator) = 1.0;
      // }

    */

    std::string RadarModel::getPowLayerName(double freq_i){
      return "P_"+getLayerName(freq_i/1e6);
    }

    std::string RadarModel::getPhaseLayerName(double freq_i){
      return "D_"+getLayerName(freq_i/1e6);
    }

    std::string RadarModel::getLayerName(double x){

        // Create an output string stream
        std::ostringstream streamObj3;

        // Set Fixed -Point Notation
        streamObj3 << std::fixed;

        // Set precision to 2 digits
        streamObj3 << std::setprecision(2);

        //Add double to stream
        streamObj3 << x;

        // Get string from output string stream
        std::string strObj3 = streamObj3.str();

        return strObj3;
    }



    /////////////////////////////////////////////////




std::vector<double> RadarModel::range(double start, double stop, double step)
{
 std::vector<double> ans;
 double currVal = start;
   while (currVal <= stop){
     ans.push_back(currVal);
     currVal += step;
   }
 return ans;
}

double RadarModel::antennaPlaneLoss(double angleRad ){
  double g_ans;
  double g_i = -22.6;
  double g_p = -22.6;
  int i;
  int p;
  double ang_i = 0;
  double ang_p = 0;
  double m = 0;
  double g_o = 0;
  double angleDeg = angleRad*180.0/M_PI;

  if (fabs(angleDeg)>180.0){
      std::cout<<"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" <<std::endl;
      std::cout<<"Angle (" << angleDeg<<") deg. " <<std::endl;
      std::cout<<"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" <<std::endl;
  }

  // gain list entries start at -180 degrees to 180 in steps of 15.
  double  fIndex = (angleDeg + 165.0) / 15.0;

  // we dont want to index out of the range ...
  i = std::max( std::min( (int) ceil(fIndex), 24), 0);
  p = std::max( std::min( (int) floor(fIndex), 24), 0);


  ang_i = (i * 15.0) - 180.0;
  ang_p = (p * 15.0) - 180.0;

  g_i = ANTENNA_LOSSES_LIST[i];
  g_p = ANTENNA_LOSSES_LIST[p];

  if (i!=p){
    m = (g_i-g_p)  / (ang_i-ang_p);
  }

  g_o = g_i - (m * ang_i);
  g_ans = m * angleDeg + g_o;

  //

  return g_ans;
}

float RadarModel::sign(float x){
  if (x > 0.0) return 1.0;
  if (x < 0.0) return -1.0;
  return 0.0;
}

void RadarModel::getSphericCoords(double x, double y, double& r, double& phi){
  r = sqrt(x*x+y*y);
  phi = atan2(y,x);

}

double RadarModel::received_power_friis(double tag_x, double tag_y, double freq, double txtPower) {
     double rxPower = txtPower;
     // build spline to interpolate antenna gains;
     std::vector<double> xVec(ANTENNA_ANGLES_LIST, ANTENNA_ANGLES_LIST + 25);
     Eigen::VectorXd xvals = Eigen::Map<Eigen::VectorXd, Eigen::Unaligned>(xVec.data(), xVec.size());
     std::vector<double> yVec(ANTENNA_LOSSES_LIST, ANTENNA_LOSSES_LIST + 25);
     Eigen::VectorXd yvals= Eigen::Map<Eigen::VectorXd, Eigen::Unaligned>(yVec.data(), yVec.size());
     SplineFunction antennaGainsModel(xvals, yvals);

     rxPower = received_power_friis(tag_x, tag_y, freq, txtPower, antennaGainsModel);
     
     return rxPower;
 }


double RadarModel::received_power_friis(double tag_x, double tag_y, double freq, double txtPower, SplineFunction antennaGainsModel) {
     double rxPower = txtPower;
     double phi;
     double r;
     // todo: use a threshold instead of exact value
     if (!((tag_x==0) && (tag_y==0))){
         getSphericCoords(tag_x,tag_y, r, phi);

         rxPower = received_power_friis_polar(r, phi, freq, txtPower, antennaGainsModel);
     }
     
     return rxPower;
 }

double RadarModel::received_power_friis_polar(double tag_r, double tag_h, double freq, double txtPower, SplineFunction antennaGainsModel) {
     double rxPower = txtPower;

     if (  !(tag_r==0) ){
         /*
          SIMPLIFICATION!!! TAG is OMNIDIRECTIONAL
          (a.k.a. don't have tag radiation pattern and
          Here they say it's ok https://www.hindawi.com/journals/ijap/2013/194145/tab4/
         */
         double ant1 = antennaGainsModel.interpRad(tag_h);

         double antL =  TAG_LOSSES + ant1;

         // propagation losses
         double propL = LOSS_CONSTANT - (20 * log10  (tag_r * freq)) ;
         // signal goes from antenna to tag and comes back again, so we double the losses
         rxPower +=  2*antL + 2*propL ;
     }
     
     return rxPower;
 }

double RadarModel::phaseDifference(double tag_x, double tag_y, double freq) {
  double phi;
  double r;

  getSphericCoords(tag_x,tag_y, r, phi);
  double phase = PHASE_CONSTANT * freq * r;
  phase = fmod(phase, M_PI);
  return phase;

}


void RadarModel::activeAreaFriis(double freq, double txtPower, double sensitivity, double distStep, double& minX, double& minY, double& maxX, double& maxY) {

  double currX = 0.0;
  double currY = 0.0;
  double currRxPower = txtPower;

  std::cout << "txtPower: " << txtPower << "\n";
  std::cout << "sensitivity: " << sensitivity << "\n";

  // build spline to interpolate antenna gains;
  std::vector<double> xVec(ANTENNA_ANGLES_LIST, ANTENNA_ANGLES_LIST + 25);
  Eigen::VectorXd xvals = Eigen::Map<Eigen::VectorXd, Eigen::Unaligned>(xVec.data(), xVec.size());
  std::vector<double> yVec(ANTENNA_LOSSES_LIST, ANTENNA_LOSSES_LIST + 25);
  Eigen::VectorXd yvals= Eigen::Map<Eigen::VectorXd, Eigen::Unaligned>(yVec.data(), yVec.size());
  SplineFunction antennaGainsModel(xvals, yvals);

  // get max X distance within sensitivity
  do{
    currX = currX+distStep;
    currRxPower = received_power_friis(currX, currY, freq, txtPower, antennaGainsModel);
  } while(currRxPower>sensitivity);
  maxX =currX-distStep; // because last iteration should be under sensitivity

  // get min X distance within sensitivity
  currX = 0.0;
  currY = 0.0;
  currRxPower = txtPower;

  do{
    currX = currX-distStep;
    currRxPower = received_power_friis(currX, currY, freq, txtPower, antennaGainsModel);
  } while(currRxPower>sensitivity);
  minX =currX+distStep; // because last iteration should be under sensitivity

  // get max Y distance within sensitivity
  currX = 0.0;
  currY = 0.0;
  currRxPower = txtPower;

  do{
    currY = currY+distStep;
    currRxPower = received_power_friis(currX, currY, freq, txtPower, antennaGainsModel);
  } while(currRxPower>sensitivity);
  maxY =currY-distStep; // because last iteration should be under sensitivity

  // get min Y distance within sensitivity
  currX = 0.0;
  currY = 0.0;
  currRxPower = txtPower;

  do{
    currY = currY-distStep;
    currRxPower = received_power_friis(currX, currY, freq, txtPower, antennaGainsModel);
  } while(currRxPower>sensitivity);
  minY =currY+distStep; // because last iteration should be under sensitivity

}



