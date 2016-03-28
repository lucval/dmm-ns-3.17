/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011 Yufei Cheng
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Yufei Cheng   <yfcheng@ittc.ku.edu>
 *
 * James P.G. Sterbenz <jpgs@ittc.ku.edu>, director
 * ResiliNets Research Group  http://wiki.ittc.ku.edu/resilinets
 * Information and Telecommunication Technology Center (ITTC)
 * and Department of Electrical Engineering and Computer Science
 * The University of Kansas Lawrence, KS USA.
 *
 * Work supported in part by NSF FIND (Future Internet Design) Program
 * under grant CNS-0626918 (Postmodern Internet Architecture),
 * NSF grant CNS-1050226 (Multilayer Network Resilience Analysis and Experimentation on GENI),
 * US Department of Defense (DoD), and ITTC at The University of Kansas.
 *
 * Notification:
 * Statistics Research, Bell Labs, Lucent Technologies and The University of North Carolina at Chapel Hill
 */


#ifndef HTTP_RANDOM_VARIABLE_H
#define HTTP_RANDOM_VARIABLE_H

#include <vector>
#include <algorithm>
#include <stdint.h>
#include <istream>
#include <ostream>

#include "ns3/object.h"
#include "ns3/attribute.h"
#include "ns3/attribute-helper.h"
#include "ns3/random-variable.h"
#include "ns3/rng-stream.h"

/**
 * \ Macro constants to use
 */
#define LOG2 0.6931471806
#define ROOT_2 1.4142135623730950488016887242096980785697 ///< 2^1/2
#define E22    1.7155277699214135929603792825575449562416 ///< sqrt(8/e)
#define HTTP_REQUEST_SIZE 0
#define HTTP_RESPONSE_SIZE 1
/*
 * \brief This random variable is based on the following paper and PackMime-ns2 Http code
 *
 * Statistics Research, Bell Labs, Lucent Technologies and The University of North Carolina at Chapel Hill
 *
 * Reference:
 * "Stochastic Models for Generating Synthetic HTTP Source Traffic"
 * Jin Cao, William S. Cleveland, Yuan Gao, Kevin Jeffay, F. Donelson Smith, Michele Weigle
 * IEEE Infocom, 2004
*/
/**
 * \ingroup http
 * \defgroup Random Variable Distributions
 *
 */
namespace ns3 {
namespace http {

struct ArimaParams
{
  double d;
  uint32_t N;
  double varRatioParam0, varRatioParam1;
  uint32_t pAR, qMA;
};

/**
 * \ingroup http
 * \brief HttpRandomVariableBase
 *
 * Reference:
 * "On the Nonstationarity of Internet Traffic"
 * J. Cao, W. S. Cleveland, D. Lin, and D. X. Sun
 * ACM SIGMETRICS, vol. 29, no. 1, pp. 102-112, 2001
 */
class HttpRandomVariableBase : public RandomVariable
{
public:
  /**
   * \brief Constructor.
   */
  HttpRandomVariableBase ();
  /**
   * \brief Destructor.
   */
  virtual ~HttpRandomVariableBase ();
  /**
   * \brief Logit one-step entropy.  log (m_x / (1 - m_x)) / LOG2
   * \param m_x the value to calculate
   * \return double the logit value
   */
  double Logit (double m_x);
  /**
   * \brief Distribution functions
   * \param m_x the value to calculate
   * \return double the logitInv value
   */
  double LogitInv (double m_x);
  /**
   * \brief Generate weibull distribution
   * \param p
   * \param shape
   * \param scale
   * \return double probability quantile function of the Weibull distribution
   */
  double qWeibull (double p, double shape, double scale);
  /**
   * \brief Generate p normal distribution
   * \param q
   * \return double the Normal value
   */
  double pNorm (double q);
  /**
   * \brief Generate p normal distribution
   * \param q
   * \return double the Normal value
   */
  uint32_t rBernoulli (double p);
  /**
   * \brief Generate the Logarithm of the Gamma function
   * \param xx
   * \return double
   */
  double Gammln (double xx);

protected:
  /**
   * Couple of random variable functions
   */
  UniformVariable m_uniform;
  WeibullVariable m_weibull;
  NormalVariable m_normal;
  GammaVariable m_gamma;
};

/**
 * \ingroup http
 * \brief slope generator based on interpolation
 */
class Slope
{
public:
  Slope (const double* x, const double* y, uint32_t n);
  ~Slope ();
  double LinearInterpolate (double xnew);
private:
  double* m_x, *m_y;
  uint32_t m_nSteps;  ///< Steps to take for linear interpolate
  double* m_slope;    ///< Avoid real-time slope computation
};

/**
 * \ingroup http
 * \brief Fractional Arima
 *
 * Reference:
 * "Modeling Persistence in Hydrological Time Series
 * Using Fractional Differencing"
 * J.R.M. Hosking
 * Water Resources Research, Vol 20, No 12, P1898-1908, Dec 1984
 * y[t] = sum_j = 1^p (AR[j] * y[t-j]) + x[t] - sum_j = 1^q (MA[j] * x[t-j]
 */
class fArimaImpl : public HttpRandomVariableBase
{
public:
  /**
   * \brief Constructor.
   */
  fArimaImpl (double d, uint32_t N, uint32_t pAR = 0, uint32_t qMA = 1);
  /**
   * \brief Destructor.
   */
  virtual ~fArimaImpl ();
  /**
   * \brief Returns next a double value from ARFIMA process
   * \return A floating point random value
   */
  double Next ();

private:
  /**
   * \brief Returns a double value from ARFIMA process
   * \return A floating point random value
   */
  double GetValue ();
  /**
   * \brief The minimum number between two numbers
   * \return An integer of the minimum value
   */
  inline uint32_t Min (uint32_t a, uint32_t b)
  {
    return ((a) < (b) ? (a) : (b));
  }
  uint32_t m_t, m_N, m_pAR, m_qMA, m_tmod;
  double* m_AR, *m_MA, *m_x, *m_y, *m_phi, m_d;
  /**
   * \brief Returns next a double value from ARFIMA process
   * \return A floating point random value
   */
  double NextLow ();
};

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// HttpFileSizeRandomVariable

class HttpFileSizeRandomVariableImpl : public HttpRandomVariableBase
{
public:
  /**
   * \brief Destructor.
   */
  HttpFileSizeRandomVariableImpl (double rate, uint32_t type);

  /**
   * \brief Destructor.
   */
  virtual ~HttpFileSizeRandomVariableImpl ();
  /**
   * \brief Returns a random double from the underlying http distribution,
   *        obtain the next file size of both http request and response
   * \return A floating point random value
   */
  double GetValue ();

private:
  /**
   * \brief Initialize the parameters and distribution functions needed
   * \return void
   */
  void Initialize ();
  /**
   * \brief Average value of each distribution function
   * \return A floating point average value
   */
  double Average ();
  /**
   *  \brief Random generate the marginal distribution of response file size
   *  \param state whether it is unmodified file (0) or content file (1)
   *  \return uint32_t the response size
   */
  uint32_t ResponseSize (uint32_t state);
  /**
   * \brief given a probability, find the corresponding quantile of fsize1.  It generate web request size
   * \param p the probability
   * \return uint32_t the request size
   */
  uint32_t RequestSize (double p);
  /**
   * Fitted inverse cdf curves for file sizes
   */
  static const double RESPONSE_INCDF_NOT_MODIFIED_X[];
  static const double RESPONSE_INCDF_NOT_MODIFIED_Y[];
  static const double RESPONSE_INCDF_CONTENT_X[];
  static const double RESPONSE_INCDF_CONTENT_Y[];
  static const double REQUEST_INCDF_NOT_MODIFIED_X[];
  static const double REQUEST_INCDF_NOT_MODIFIED_Y[];
  static const double REQUEST_INCDF_CONTENT_X[];
  static const double REQUEST_INCDF_CONTENT_Y[];
  static const double REQUEST_PROB_A;      ///< This is the request probability
  static const double REQUEST_D;           ///< This is the FSD model parameter d, the estimate of d for file size is 0.31
  /**
   * Modeled by an FSD model.  The estimated value of d is 0.31 and the logistic model is
   * LogitInv (0.123 + 0.494 * log (m_rate) / LOG2)
   * This is used to generate request file
   */
  static const double REQUEST_VARRATIO_INTERCEPT;
  static const double REQUEST_VARRATIO_SLOPE;
//  /**
//   * Server file size run length for downloaded and cache validation
//   */
//  static const double WEIBULLSCALECACHERUN;
//  static const double WEIBULLSHAPECACHERUN_ASYMPTOE;
//  static const double WEIBULLSHAPECACHERUN_PARA1;
//  static const double WEIBULLSHAPECACHERUN_PARA2;
//  static const double WEIBULLSHAPECACHERUN_PARA3;
//  static const double WEIBULLSCALEDOWNLOADRUN;
//  static const double WEIBULLSHAPEDOWNLOADRUN;
//  static const double RESPONSE_PARA[];
  static struct ArimaParams fileSizeArimaParams;
//  static const double* P;
  static Slope fsize_invcdf[2][2];
//  /**
//   * cut off of file size for cache validation
//   */
//  static const uint32_t RESPONSE_CACHE_CUTOFF;
//  static const uint32_t RESPONSE_STRETCH_THRES;
//  /**
//   * mean of log2(fsize0) for non cache validation flows
//   */
//  static const double M_RESPONSE_NOTCACHE;
//  /**
//   * variance of log2(fsize0) for non cache validation flows
//   */
//  static const double V_RESPONSE_NOTCACHE;
//  /**
//   * fsize0 for non-cache validation flow
//   */
//  static const double M_LOC;        /// mean of location
//  static const double V_LOC;        /// variance of location
//  static const double SHAPE_SCALE2; /// Gamma shape parameter of scale^2
//  static const double RATE_SCALE2;  /// Gamma rate parameter of scale^2
//  static const double V_ERROR;      /// variance of error

  double m_rate;
  uint32_t m_type;                ///< This is the type of file size, (e.g. Web request or web response)
  double m_const;
  double m_mean;
  double m_varRatio, m_sigmaNoise, m_sigmaEpsilon;
  uint32_t m_runlen, m_state;
  double m_shape[2], m_scale[2];
  double m_loc;
  double m_scale2;
  fArimaImpl* m_fArima;
};

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// HttpNumPagesRandomVariable

class HttpNumPagesRandomVariableImpl : public HttpRandomVariableBase
{
public:
  /**
   * \brief Constructor.
   */
  HttpNumPagesRandomVariableImpl (double prob, double shape, double scale);
  /**
   * \brief Destructor.
   */
  virtual ~HttpNumPagesRandomVariableImpl ();
  /**
   * \brief Returns a random double from the underlying http distribution
   * \return A floating point random value
   */
  double GetValue ();
  /// The probability that number of pages for one transaction is only 1
  static const double P_1PAGE;
  /**
   *  The shape and scale parameter are for discrete Weibull function,
   *  however, the scale parameter here is different from the one in paper
   */
  static const double SHAPE_NPAGE;   ///< shape param for (#page reqs-1)
  static const double SCALE_NPAGE;   ///< scale param for (#page reqs-1)

private:
  /**
   * \brief Average value of each distribution function
   * \return A floating point average value
   */
  double Average ();

  double m_probability;              ///< probability that the connection has just one page
  double m_shape;                    ///< shape of Weibull for number of pages in connection
  double m_scale;                    ///< scale of Weibull for number of pages in connection
};

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// HttpSingleObjRandomVariable

class HttpSingleObjRandomVariableImpl : public HttpRandomVariableBase
{
public:
  /**
   * \brief Constructor.
   */
  HttpSingleObjRandomVariableImpl ();
  HttpSingleObjRandomVariableImpl (double prob);
  /**
   * \brief Destructor.
   */
  virtual ~HttpSingleObjRandomVariableImpl ();
  /**
   * \brief Returns a random double from the underlying http distribution
   * \return A floating point random value
   */
  double GetValue ();
  /**
   * The probability that the web page only has one web page, and the value is 0.69
   */
  static const double P_1TRANSFER;      ///< P(#xfers=1 | #page req>=2)
private:
  /**
   * \brief Average value of each distribution function
   * \return A floating point average value
   */
  double Average ();
  double m_probability;    ///< probability that the there is only one object in the web page
};

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// HttpObjsPerPageRandomVariable

class HttpObjsPerPageRandomVariableImpl : public HttpRandomVariableBase
{
public:
  /**
   * \brief Constructor.
   */
  HttpObjsPerPageRandomVariableImpl ();
  HttpObjsPerPageRandomVariableImpl (double shape, double scale);
  /**
   * \brief Destructor.
   */
  virtual ~HttpObjsPerPageRandomVariableImpl ();
  /**
   * \brief Returns a random double from the underlying http distribution
   * \return A floating point random value
   */
  double GetValue ();

  static const double SHAPE_NTRANSFER;          ///< shape param for (#xfers-1)
  static const double SCALE_NTRANSFER;          ///< scale param for (#xfers-1) 1.578

private:
  /**
   * \brief Average value of each distribution function
   * \return A floating point average value
   */
  double Average ();
  double m_shape;          ///< shape of Weibull for number of objs in page
  double m_scale;          ///< scale of Weibull for number of objs in page
};

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// HttpTimeBtwnPagesRandomVariable

class HttpTimeBtwnPagesRandomVariableImpl  : public HttpRandomVariableBase
{
public:
  /**
   * \brief Constructor.
   */
  HttpTimeBtwnPagesRandomVariableImpl ();
  /**
   * \brief Destructor.
   */
  virtual ~HttpTimeBtwnPagesRandomVariableImpl ();
  /**
   * \brief Returns a random double from the underlying http distribution
   * \return A floating point random value
   */
  double GetValue ();
  /**
   * \brief Time gap between page requests, the value of each variable is based on real world trace
   *        this is one mixed-effects model with random location and scale effects
   * Reference:
   * "Random Scale Effects"
   * W. S. Cleveland, L. Denby, and C. Liu
   * Bell Labs, stat.bell-labs.com, Tech. Rep., 2003
   */
  static const double M_LOC_B;                  ///< Mean of location
  static const double V_LOC_B;                  ///< Variance of location
  static const double SHAPE_SCALE2_B;           ///< Gamma shape param of scale^2
  static const double RATE_SCALE2_B;            ///< Gamma rate param of scale^2
  static const double V_ERROR_B;                ///< Variance of error

private:
  /**
   * \brief Average value of each distribution function
   * \return A floating point average value
   */
  double Average ();
  double m_loc_b;
  double m_scale2_b;
};

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// HttpTimeBtwnObjsRandomVariable

class HttpTimeBtwnObjsRandomVariableImpl  : public HttpRandomVariableBase
{
public:
  /**
   * \brief Constructor.
   */
  HttpTimeBtwnObjsRandomVariableImpl ();
  /**
   * \brief Destructor.
   */
  virtual ~HttpTimeBtwnObjsRandomVariableImpl ();
  /**
   * \brief Returns a random double from the underlying http distribution
   * \return A floating point random value
   */
  double GetValue ();
  /**
   * \brief Time gap between objects within one web page, the value of each variable is based on real world trace
   *        this is one mixed-effects model with random location and scale effects
   * Reference:
   * "Random Scale Effects"
   * W. S. Cleveland, L. Denby, and C. Liu
   * Bell Labs, stat.bell-labs.com, Tech. Rep., 2003
   */
  static const double M_LOC_W;        ///< mean of location
  static const double V_LOC_W;        ///< variance of location
  static const double SHAPE_SCALE2_W; ///< Gamma shape param of scale^2
  static const double RATE_SCALE2_W;  ///< Gamma rate param of scale^2
  static const double V_ERROR_W;      ///< variance of error

private:
  /**
   * \brief Average value of each distribution function
   * \return A floating point average value
   */
  double Average ();
  double m_loc_w;
  double m_scale2_w;
};

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// HttpServerDelayRandomVariable

class HttpServerDelayRandomVariableImpl : public HttpRandomVariableBase
{
public:
  /**
   * \brief Constructor.
   */
  HttpServerDelayRandomVariableImpl (double shape, double scale);
  /**
   * \brief Destructor.
   */
  virtual ~HttpServerDelayRandomVariableImpl ();
  /**
   * \brief Returns a random double from the underlying http distribution
   * \return A floating point random value
   */
  virtual double GetValue ();
  /**
   * \brief Average value of each distribution function
   * \return A floating point average value
   */
  virtual double Average ();
  /**
   * Weibull distribution based on BELL data is shape = 0.63, scale = 305
   */
  static const double SERVER_DELAY_SHAPE;           ///< The shape of the weibull distribution
  static const double SERVER_DELAY_SCALE;           ///< The scale of the weibull
  static const double SERVER_DELAY_DIV;             ///< The deviation of server delay

private:
  double m_shape;
  double m_scale;
  double m_const;
  double m_mean;
};
} // namespace http
} // namespace ns3

#endif
