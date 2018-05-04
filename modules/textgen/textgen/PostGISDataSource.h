// ======================================================================
/*!
 * \file
 * \brief Interface of class BrainStorm::PostGISDataSource
 */
// ======================================================================

#ifndef POSTGIS_DATA_SOURCE_H
#define POSTGIS_DATA_SOURCE_H

#include <list>
#include <string>
#include <map>

class OGRDataSource;

namespace BrainStorm
{
struct postgis_identifier
{
  std::string postGISHost;
  std::string postGISPort;
  std::string postGISDatabase;
  std::string postGISUsername;
  std::string postGISPassword;
  std::string postGISSchema;
  std::string postGISTable;
  std::string postGISField;
  std::string postGISClientEncoding;
  std::string key()
  {
    return (postGISHost + ";" + postGISPort + ";" + postGISDatabase + ";" + postGISUsername + ";" +
            postGISPassword + ";" + postGISSchema + ";" + postGISTable + ";" + postGISField + ";" +
            postGISClientEncoding);
  }
  bool allFieldsDefined()
  {
    return (!postGISHost.empty() && !postGISPort.empty() && !postGISDatabase.empty() &&
            !postGISUsername.empty() && !postGISPassword.empty() && !postGISSchema.empty() &&
            !postGISTable.empty() && !postGISField.empty() && !postGISClientEncoding.empty());
  }
};

//  class WeatherArea;

class PostGISDataSource
{
 public:
  PostGISDataSource() {}
  bool readData(const std::string& host,
                const std::string& port,
                const std::string& dbname,
                const std::string& user,
                const std::string& password,
                const std::string& schema,
                const std::string& table,
                const std::string& fieldname,
                const std::string& client_encoding,
                std::string& log_message);

  bool readData(const postgis_identifier& postGISIdentifier, std::string& log_message);

  bool geoObjectExists(const std::string& name) const;
  bool isPolygon(const std::string& name) const
  {
    return polygonmap.find(name) != polygonmap.end();
  }
  bool isLine(const std::string& name) const { return linemap.find(name) != linemap.end(); }
  bool isPoint(const std::string& name) const { return pointmap.find(name) != pointmap.end(); }
  std::string getSVGPath(const std::string& name) const;
  std::pair<double, double> getPoint(const std::string& name) const;

  void resetQueryParameters() { queryparametermap.clear(); }
  std::list<std::string> areaNames() const;

 private:
  OGRDataSource* connect(const std::string& host,
                         const std::string& port,
                         const std::string& dbname,
                         const std::string& user,
                         const std::string& password);

  std::map<std::string, std::string> polygonmap;
  std::map<std::string, std::string> linemap;
  std::map<std::string, std::pair<double, double> > pointmap;

  std::map<std::string, int> queryparametermap;

};  // class PostGISDataSource
}  // namespace BrainStorm

#endif  // POSTGIS_DATA_SOURCE_H

// ======================================================================
