/* Copyright 2008 (C) Nicira, Inc.
 *
 * This file is part of NOX.
 *
 * NOX is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * NOX is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with NOX.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef FLOW_HH
#define FLOW_HH 1

#include <iostream>
#include <string>
#include <map>
#include <cstring>
#include <iosfwd>
#include "netinet++/ethernetaddr.hh"
#include "openflow/openflow.h"
//#include "openflow-pack-raw.hh"
#include "../oflib/ofl-structs.h"
#include "../oflib/oxm-match.h"

namespace vigil {

class Buffer;

class Flow {
public:
  struct ofl_match match;
    
  /** Empty constructor
   */
  Flow();
  /** Copy constructor
   */
  Flow(const Flow& flow_);
  /** Constructor from ofl_match
   */
  Flow(const struct ofl_match *match);
  /** Add an OXM TLV to the match
   */
  template<typename T> 
  void Add_Field(std::string name, T value){

    if(!fields.count(name))
       std::cout <<"Match field: "<< name << " is not supported "<< std::endl;
    else {
       ofl_structs_match_put(&this->match, fields[name].first, value);
    }
  }
  
  /** Add a OXM TLV to the match 
   *  with a masked value
   */  
  template<typename T> 
  void Add_Field(std::string name, T value, T mask){
    if(!fields.count(name))
       std::cout <<"Match field: "<< name << " is not supported "<< std::endl;
    else {
       ofl_structs_match_put_masked(&this->match, fields[name].first, value, mask);
    }
  }

  void Add_Field(std::string name, uint8_t value[ETH_ADDR_LEN]){

    if(!fields.count(name))
       std::cout <<"Match field: "<< name << " is not supported "<< std::endl;
    else {
       ofl_structs_match_put_eth(&this->match, fields[name].first, value);
    }
  }

  void Add_Field(std::string name, uint8_t value[ETH_ADDR_LEN],uint8_t  mask[ETH_ADDR_LEN]){

    if(!fields.count(name))
       std::cout <<"Match field: "<< name << " is not supported "<< std::endl;
    else {
       ofl_structs_match_put_eth_m(&this->match, fields[name].first, value, mask);
    }
  }
 
  
  template<typename T>
  void get_Field(std::string name, T *value ){
     struct ofl_match_tlv *omt;
     HMAP_FOR_EACH_WITH_HASH(omt, struct ofl_match_tlv, hmap_node, hash_int(fields[name].first, 0),
          &match.match_fields){
          memcpy(value, omt->value, sizeof(T));
          return;   
     }
     /* Field is not present in the packet */
     *value = -1;
  }
  
  void get_Field(std::string name, uint8_t  value[ETH_ADDR_LEN] ){
     struct ofl_match_tlv *omt;
     HMAP_FOR_EACH_WITH_HASH(omt, struct ofl_match_tlv, hmap_node, hash_int(fields[name].first, 0),
          &match.match_fields){
		  memcpy(value, omt->value, ETH_ADDR_LEN);
          return;   
     }
     /* Field is not present in the packet */
     *value = -1;
  }
  /** \brief String representation
   */
  const std::string to_string() const;
  /** \brief Return hash code
   */
  uint64_t hash_code() const;
private:
  void init();
};
bool operator==(const Flow& lhs, const Flow& rhs);
bool operator!=(const Flow& lhs, const Flow& rhs);
std::ostream& operator<<(std::ostream&, const Flow&);

template<> inline
void Flow::Add_Field<std::string>(std::string name, std::string value){

    if(!fields.count(name))
       std::cout <<"Match field: "<< name << " is not supported "<< std::endl;
    else {
        if(fields[name].second ==  ETH_ADDR_LEN){
            ethernetaddr addr = ethernetaddr(value);
            ofl_structs_match_put_eth(&this->match, fields[name].first, addr.octet);
        }
        else {
            struct in6_addr addr;
            inet_pton(AF_INET6, value.c_str(), &addr); 
            ofl_structs_match_put(&this->match, fields[name].first, addr);
        }
    }
}

} // namespace vigil

ENTER_HASH_NAMESPACE
template <>
struct hash<vigil::Flow> {
  std::size_t operator() (const vigil::Flow& flow) const {
    return HASH_NAMESPACE::hash<uint64_t>()(flow.hash_code());
  }
};
EXIT_HASH_NAMESPACE

#endif /* flow.hh */
