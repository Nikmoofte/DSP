#ifndef WAVCONTROLLER_HPP
#define WAVCONTROLLER_HPP

#include <vector>
#include <string>

class WAVController
{
public: 
    static void PlaylayWAV(const std::vector<float>& data);
    static void CreateWAVFile(std::string&& name, const std::vector<float>& data);
private:


};

#endif