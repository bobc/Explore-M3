/* Copyright (c) 2010-2011 mbed.org, MIT License
*
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software
* and associated documentation files (the "Software"), to deal in the Software without
* restriction, including without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all copies or
* substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
* BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
* NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
* DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#if defined(TARGET_LPC1768) || defined (__LPC17XX__)

// void setled(int, bool);
//#define setled(a, b) do {} while (0)

#include <LPC17xx.h>

#include "USBHAL.h"

// #define DEBUG 1
#include "debug.h"


#ifdef MBED
    #include <score_cm3.h>
#else
//!    #include <lpc17xx_nvic.h>
//    #include <lpc17xx.h>
#endif

//!#include "wait_api.h"

extern "C" { void delay_ms(int); }

// Get endpoint direction
#define IN_EP(endpoint)     ((endpoint) & 1U ? true : false)
#define OUT_EP(endpoint)    ((endpoint) & 1U ? false : true)

#define  IN_BEP(endpoint)    ((endpoint) & 0x80 ? true : false)
#define OUT_BEP(endpoint)    ((endpoint) & 0x80 ? false : true)

// Convert physical endpoint number to register bit
#define EP(endpoint) (1UL<<endpoint)

#define ISOCHRONOUS_ENDPOINTS ((1UL << 3) | (1UL << 6) | (1UL << 9) | (1UL << 12))

#define IS_ISOCHRONOUS(bEP) ((1UL << (bEP & 0x0F)) & ISOCHRONOUS_ENDPOINTS)

// Power Control for Peripherals register
#define PCUSB      (1UL<<31)

// USB Clock Control register
#define DEV_CLK_EN (1UL<<1)
#define AHB_CLK_EN (1UL<<4)

// USB Clock Status register
#define DEV_CLK_ON (1UL<<1)
#define AHB_CLK_ON (1UL<<4)

// USB Device Interupt registers
#define FRAME      (1UL<<0)
#define EP_FAST    (1UL<<1)
#define EP_SLOW    (1UL<<2)
#define DEV_STAT   (1UL<<3)
#define CCEMPTY    (1UL<<4)
#define CDFULL     (1UL<<5)
#define RxENDPKT   (1UL<<6)
#define TxENDPKT   (1UL<<7)
#define EP_RLZED   (1UL<<8)
#define ERR_INT    (1UL<<9)

/* USBRxPLen bits */
#define PKT_LNGTH                   (1<<0)
#define PKT_LNGTH_MASK              0x3FF
#define DV                          (1<<10)
#define PKT_RDY                     (1<<11)

/* Select Endpoint command read bits */
#define EPSTAT_FE                   (1<<0)
#define EPSTAT_ST                   (1<<1)
#define EPSTAT_STP                  (1<<2)
#define EPSTAT_PO                   (1<<3)
#define EPSTAT_EPN                  (1<<4)
#define EPSTAT_B1FULL               (1<<5)
#define EPSTAT_B2FULL               (1<<6)

// endpoint status sent through callback
#define EP_STATUS_DATA      (1<<0)      /**< EP has data */
#define EP_STATUS_STALLED   (1<<1)      /**< EP is stalled */
#define EP_STATUS_SETUP     (1<<2)      /**< EP received setup packet */
#define EP_STATUS_ERROR     (1<<3)      /**< EP data was overwritten by setup packet */
#define EP_STATUS_NACKED    (1<<4)      /**< EP sent NAK */

// USB Control register
#define RD_EN (1<<0)
#define WR_EN (1<<1)
#define LOG_ENDPOINT(endpoint) ((endpoint>>1)<<2)

// USB Receive Packet Length register
// #define DV      (1UL<<10)
// #define PKT_RDY (1UL<<11)
// #define PKT_LNGTH_MASK (0x3ff)

// Serial Interface Engine (SIE)
#define SIE_WRITE   (0x01)
#define SIE_READ    (0x02)
#define SIE_COMMAND (0x05)
#define SIE_CMD_CODE(phase, data) ((phase<<8)|(data<<16))

// SIE Command codes
#define SIE_CMD_SET_ADDRESS        (0xD0)
#define SIE_CMD_CONFIGURE_DEVICE   (0xD8)
#define SIE_CMD_SET_MODE           (0xF3)
#define SIE_CMD_READ_FRAME_NUMBER  (0xF5)
#define SIE_CMD_READ_TEST_REGISTER (0xFD)
#define SIE_CMD_SET_DEVICE_STATUS  (0xFE)
#define SIE_CMD_GET_DEVICE_STATUS  (0xFE)
#define SIE_CMD_GET_ERROR_CODE     (0xFF)
#define SIE_CMD_READ_ERROR_STATUS  (0xFB)

#define SIE_CMD_SELECT_ENDPOINT(endpoint)                 (0x00+endpoint)
#define SIE_CMD_SELECT_ENDPOINT_CLEAR_INTERRUPT(endpoint) (0x40+endpoint)
#define SIE_CMD_SET_ENDPOINT_STATUS(endpoint)             (0x40+endpoint)

#define SIE_CMD_CLEAR_BUFFER    (0xF2)
#define SIE_CMD_VALIDATE_BUFFER (0xFA)

// SIE Device Status register
#define SIE_DS_CON    (1<<0)
#define SIE_DS_CON_CH (1<<1)
#define SIE_DS_SUS    (1<<2)
#define SIE_DS_SUS_CH (1<<3)
#define SIE_DS_RST    (1<<4)

// SIE Device Set Address register
#define SIE_DSA_DEV_EN  (1<<7)

// SIE Configue Device register
#define SIE_CONF_DEVICE (1<<0)

// Select Endpoint register
#define SIE_SE_FE       (1<<0)
#define SIE_SE_ST       (1<<1)
#define SIE_SE_STP      (1<<2)
#define SIE_SE_PO       (1<<3)
#define SIE_SE_EPN      (1<<4)
#define SIE_SE_B_1_FULL (1<<5)
#define SIE_SE_B_2_FULL (1<<6)

// Set Endpoint Status command
#define SIE_SES_ST      (1<<0)
#define SIE_SES_DA      (1<<5)
#define SIE_SES_RF_MO   (1<<6)
#define SIE_SES_CND_ST  (1<<7)

// endpoint modes
#define SIE_MODE_AP_CLK     (1<<0)
#define SIE_MODE_INAK_CI    (1<<1)
#define SIE_MODE_INAK_CO    (1<<2)
#define SIE_MODE_INAK_II    (1<<3)
#define SIE_MODE_INAK_IO    (1<<4)
#define SIE_MODE_INAK_BI    (1<<5)
#define SIE_MODE_INAK_BO    (1<<6)

USBHAL * USBHAL::instance;

static volatile uint32_t epComplete;

//static volatile uint32_t USBEpIntEn;
static uint32_t endpointStallState;

static void SIECommand(uint32_t command) {
    // The command phase of a SIE transaction
    LPC_USB->USBDevIntClr = CCEMPTY;
    LPC_USB->USBCmdCode = SIE_CMD_CODE(SIE_COMMAND, command);
    while (!(LPC_USB->USBDevIntSt & CCEMPTY));
}

static void SIEWriteData(uint8_t data) {
    // The data write phase of a SIE transaction
    LPC_USB->USBDevIntClr = CCEMPTY;
    LPC_USB->USBCmdCode = SIE_CMD_CODE(SIE_WRITE, data);
    while (!(LPC_USB->USBDevIntSt & CCEMPTY));
}

static uint8_t SIEReadData(uint32_t command) {
    // The data read phase of a SIE transaction
    LPC_USB->USBDevIntClr = CDFULL;
    LPC_USB->USBCmdCode = SIE_CMD_CODE(SIE_READ, command);
    while (!(LPC_USB->USBDevIntSt & CDFULL));
    return (uint8_t)LPC_USB->USBCmdData;
}

static void SIEsetDeviceStatus(uint8_t status) {
    // Write SIE device status register
    SIECommand(SIE_CMD_SET_DEVICE_STATUS);
    SIEWriteData(status);
}

static uint8_t SIEgetDeviceStatus(void) {
    // Read SIE device status register
    SIECommand(SIE_CMD_GET_DEVICE_STATUS);
    return SIEReadData(SIE_CMD_GET_DEVICE_STATUS);
}

static void SIEsetMode(uint8_t mode) {
    SIECommand(SIE_CMD_SET_MODE);
    SIEWriteData(mode);
}

static void SIEsetAddress(uint8_t address) {
    // Write SIE device address register
    SIECommand(SIE_CMD_SET_ADDRESS);
    SIEWriteData((address & 0x7f) | SIE_DSA_DEV_EN);
}

static uint8_t SIEselectEndpoint(uint8_t bEP) {
    uint8_t endpoint = EP2IDX(bEP);

    // SIE select endpoint command
    SIECommand(SIE_CMD_SELECT_ENDPOINT(endpoint));
    return SIEReadData(SIE_CMD_SELECT_ENDPOINT(endpoint));
}

static uint8_t SIEclearBuffer(void) {
    // SIE clear buffer command
    SIECommand(SIE_CMD_CLEAR_BUFFER);
    return SIEReadData(SIE_CMD_CLEAR_BUFFER);
}

static void SIEvalidateBuffer(void) {
    // SIE validate buffer command
    SIECommand(SIE_CMD_VALIDATE_BUFFER);
}

static void SIEsetEndpointStatus(uint8_t bEP, uint8_t status) {
    uint8_t endpoint = EP2IDX(bEP);

    // SIE set endpoint status command
    SIECommand(SIE_CMD_SET_ENDPOINT_STATUS(endpoint));
    SIEWriteData(status);
}

static uint16_t SIEgetFrameNumber(void) __attribute__ ((unused));
static uint16_t SIEgetFrameNumber(void) {
    // Read current frame number
    uint16_t lowByte;
    uint16_t highByte;

    SIECommand(SIE_CMD_READ_FRAME_NUMBER);
    lowByte = SIEReadData(SIE_CMD_READ_FRAME_NUMBER);
    highByte = SIEReadData(SIE_CMD_READ_FRAME_NUMBER);

    return (highByte << 8) | lowByte;
}

static void SIEconfigureDevice(void) {
    // SIE Configure device command
    SIECommand(SIE_CMD_CONFIGURE_DEVICE);
    SIEWriteData(SIE_CONF_DEVICE);
}

static void SIEunconfigureDevice(void) {
    // SIE Configure device command
    SIECommand(SIE_CMD_CONFIGURE_DEVICE);
    SIEWriteData(0);
}

static void SIEconnect(void) {
    // Connect USB device
    uint8_t status;

    status = SIEgetDeviceStatus();
    SIEsetDeviceStatus(status | SIE_DS_CON);
}


static void SIEdisconnect(void) {
    // Disconnect USB device
    uint8_t status;

    status = SIEgetDeviceStatus();
    SIEsetDeviceStatus(status & ~SIE_DS_CON);
}


static uint8_t selectEndpointClearInterrupt(uint8_t bEP) {
    uint8_t endpoint = EP2IDX(bEP);

    // Implemented using using EP_INT_CL
    LPC_USB->USBEpIntClr = EP(endpoint);
    while (!(LPC_USB->USBDevIntSt & CDFULL));
    return (uint8_t)LPC_USB->USBCmdData;
}

static void enableEndpointInt (uint8_t bEP) {
    uint8_t endpoint = EP2IDX(bEP);

    // Enable an endpoint interrupt
    LPC_USB->USBEpIntEn |= EP(endpoint);
}

static void disableEndpointInt (uint8_t bEP) __attribute__ ((unused));
static void disableEndpointInt (uint8_t bEP) {
    uint8_t endpoint = EP2IDX(bEP);

    // Disable an endpoint interrupt
    LPC_USB->USBEpIntEn &= ~EP(endpoint);
}

static volatile uint32_t __attribute__((used)) dummyRead;
uint32_t USBHAL::endpointReadcore(uint8_t bEP, uint8_t *buffer) {
    // Read from an OUT endpoint
    uint32_t size;
    uint32_t i;
    uint32_t data = 0;
    uint8_t offset;
    uint8_t endpoint = EP2IDX(bEP);

    uint8_t irq = NVIC_GetActive(USB_IRQn);
//!    NVIC_DisableIRQ(USB_IRQn);

    LPC_USB->USBCtrl = LOG_ENDPOINT(endpoint) | RD_EN;
    while (!(LPC_USB->USBRxPLen & PKT_RDY));

    size = LPC_USB->USBRxPLen & PKT_LNGTH_MASK;

    if ((IS_ISOCHRONOUS(bEP) == 0) && (size > 64)) {
        size = 64;
    }

    offset = 0;
    if (size > 0) {
        for (i = 0; i < size; i++) {
            if (offset==0) {
                // Fetch up to four bytes of data as a word
                data = LPC_USB->USBRxData;
            }

            // extract a byte
            *buffer = (data>>offset) & 0xff;
            buffer++;

            // move on to the next byte
            offset = (offset + 8) & 24;
        }
    } else {
        dummyRead = LPC_USB->USBRxData;
    }

    while ((LPC_USB->USBDevIntSt & RxENDPKT) == 0)
        dummyRead = LPC_USB->USBRxData;

    LPC_USB->USBCtrl = 0;

    if (IS_ISOCHRONOUS(bEP) == 0) {
        SIEselectEndpoint(bEP);
        SIEclearBuffer();
    }

    return size;
}

static void endpointWritecore(uint8_t bEP, uint8_t *buffer, uint32_t size)
{
    // Write to an IN endpoint
    uint8_t endpoint = EP2IDX(bEP);

    LPC_USB->USBCtrl = LOG_ENDPOINT(endpoint) | WR_EN;

    LPC_USB->USBTxPLen = size;
    while (LPC_USB->USBCtrl & WR_EN)
    {
        LPC_USB->USBTxData = (buffer[3] << 24) | (buffer[2] << 16) | (buffer[1] << 8) | buffer[0];
        buffer += 4;
    }

    // Clear WR_EN to cover zero length packet case
    LPC_USB->USBCtrl = 0;

    SIEselectEndpoint(bEP);
    SIEvalidateBuffer();

    //todo: is this needed?
    //uint8_t status __attribute__ ((unused)) = SIEselectEndpoint(bEP);
}


USBHAL::USBHAL(void) {
    instance = this;
}

void USBHAL::init() {
    // Disable IRQ
    NVIC_DisableIRQ(USB_IRQn);

    // Enable power to USB device controller
    LPC_SC->PCONP |= PCUSB;

    // Enable USB clocks
    LPC_USB->USBClkCtrl |= DEV_CLK_EN | AHB_CLK_EN;
    while (LPC_USB->USBClkSt != (DEV_CLK_ON | AHB_CLK_ON));

    // Configure pins P0.29 and P0.30 to be USB D+ and USB D-
    LPC_PINCON->PINSEL1 &= 0xc3ffffff;
    LPC_PINCON->PINSEL1 |= 0x14000000;

    // Configure pin P2.9 to be Connect
    LPC_PINCON->PINSEL4 &= 0xfffcffff;
    LPC_PINCON->PINSEL4 |= 0x00040000;

    // Disconnect USB device
    SIEdisconnect();

    // Connect must be low for at least 2.5uS
    // work around OSX behaviour where if the device disconnects and quickly reconnects, it assumes it's the same device instead of checking
//!    wait_ms(1000);
    delay_ms(100);

    // Set the maximum packet size for the control endpoints
    realiseEndpoint(IDX2EP(EP0IN), MAX_PACKET_SIZE_EP0, 0);
    realiseEndpoint(IDX2EP(EP0OUT), MAX_PACKET_SIZE_EP0, 0);

    // Attach IRQ
    instance = this;
    // NVIC_SetVector(USB_IRQn, (uint32_t)&_usbisr);
    // NVIC_EnableIRQ(USB_IRQn);


    // Enable interrupts for device events and EP0
    LPC_USB->USBDevIntEn = EP_SLOW | DEV_STAT | FRAME;
    //LPC_USB->USBDevIntEn = EP_SLOW | DEV_STAT;
    //USBEpIntEn = 0x3;
    enableEndpointInt(IDX2EP(EP0IN));
    enableEndpointInt(IDX2EP(EP0OUT));
}

USBHAL::~USBHAL(void) {
    // Ensure device disconnected
    SIEdisconnect();

    // Disable USB interrupts
    NVIC_DisableIRQ(USB_IRQn);
}

uint32_t USBHAL::getSerialNumber(int length, uint32_t *buf) {
    #define IAP_LOCATION 0x1FFF1FF1
    uint32_t command[1];
    uint32_t result[5];
    typedef void (*IAP)(uint32_t*, uint32_t*);
    IAP iap = (IAP) IAP_LOCATION;

    __disable_irq();
    command[0] = 58;
    iap(command, result);
    int i;
    for (i = 0; i < 4; i++) {
        if (i < length) {
            buf[i] = result[i + 1];
        }
    }
    __enable_irq();
    return i;
}

void USBHAL::connect(void) {
    NVIC_EnableIRQ(USB_IRQn);
    // Connect USB device
    SIEconnect();
}

void USBHAL::disconnect(void) {
    NVIC_DisableIRQ(USB_IRQn);
    // Disconnect USB device
    SIEdisconnect();
}

void USBHAL::configureDevice(void) {
    SIEconfigureDevice();
}

void USBHAL::unconfigureDevice(void) {
    SIEunconfigureDevice();
}

void USBHAL::setAddress(uint8_t address) {
    SIEsetAddress(address);
//1     SIEsetMode(SIE_MODE_INAK_CI | SIE_MODE_INAK_CO | SIE_MODE_INAK_BI | SIE_MODE_INAK_BO);
//2    SIEsetMode(SIE_MODE_INAK_CI | SIE_MODE_INAK_CO);
}

void USBHAL::EP0setup(uint8_t *buffer) {
    endpointReadcore(IDX2EP(EP0OUT), buffer);
}

void USBHAL::EP0read(void) {
    // Not required
}

uint32_t USBHAL::EP0getReadResult(uint8_t *buffer) {
    return endpointReadcore(IDX2EP(EP0OUT), buffer);
}

void USBHAL::EP0write(uint8_t *buffer, uint32_t size) {
    endpointWritecore(IDX2EP(EP0IN), buffer, size);
}

void USBHAL::EP0getWriteResult(void) {
    // Not required
}

void USBHAL::EP0stall(void) {
    // This will stall both control endpoints
    stallEndpoint(IDX2EP(EP0OUT));
}

EP_STATUS USBHAL::endpointRead(uint8_t bEP, uint32_t maximumSize) {
    return EP_PENDING;
}

EP_STATUS USBHAL::endpointReadResult(uint8_t bEP, uint8_t * buffer, uint32_t *bytesRead)
{
    uint8_t endpoint = EP2IDX(bEP);

    //for isochronous endpoint, we don't wait an interrupt
    if (IS_ISOCHRONOUS(bEP) == 0) {
//         iprintf("not Isochronous\n");
         if (!(epComplete & EP(endpoint)))
            return EP_PENDING;
    }

    *bytesRead = endpointReadcore(bEP, buffer);
    epComplete &= ~EP(endpoint);

    return EP_COMPLETED;
}

EP_STATUS USBHAL::endpointWrite(uint8_t bEP, uint8_t *data, uint32_t size) {
    uint8_t endpoint = EP2IDX(bEP);

    if (getEndpointStallState(bEP)) {
        return EP_STALLED;
    }

    epComplete &= ~EP(endpoint);

    endpointWritecore(bEP, data, size);
    return EP_PENDING;
}

EP_STATUS USBHAL::endpointWriteResult(uint8_t bEP)
{
    uint8_t endpoint = EP2IDX(bEP);

    if (epComplete & EP(endpoint)) {
        epComplete &= ~EP(endpoint);  //??
        return EP_COMPLETED;
    }

    return EP_PENDING;
}

uint8_t USBHAL::endpointStatus(uint8_t bEP)
{
    uint8_t bEPStat = SIEselectEndpoint(EP2IDX(bEP));

    uint8_t bStat __attribute__ ((unused))
                  = ((bEPStat & EPSTAT_FE ) ? EP_STATUS_DATA    : 0) |
                    ((bEPStat & EPSTAT_ST ) ? EP_STATUS_STALLED : 0) |
                    ((bEPStat & EPSTAT_STP) ? EP_STATUS_SETUP   : 0) |
                    ((bEPStat & EPSTAT_EPN) ? EP_STATUS_NACKED  : 0) |
                    ((bEPStat & EPSTAT_PO ) ? EP_STATUS_ERROR   : 0);

    return bEPStat;
}

bool USBHAL::realiseEndpoint(uint8_t bEP, uint32_t maxPacket, uint32_t flags)
{
    uint8_t endpoint = EP2IDX(bEP);

    // Realise an endpoint
    LPC_USB->USBDevIntClr = EP_RLZED;
    LPC_USB->USBReEp |= EP(endpoint);
    LPC_USB->USBEpInd = endpoint;
    LPC_USB->USBMaxPSize = maxPacket;

    while (!(LPC_USB->USBDevIntSt & EP_RLZED));
    LPC_USB->USBDevIntClr = EP_RLZED;

    // Clear stall state
//     endpointStallState &= ~EP(endpoint);
    unstallEndpoint(bEP);

    enableEndpointInt(bEP);

    return true;
}

void USBHAL::stallEndpoint(uint8_t bEP)
{
    uint8_t endpoint = EP2IDX(bEP);

    // Stall an endpoint
    if ( (endpoint==EP0IN) || (endpoint==EP0OUT) ) {
        // Conditionally stall both control endpoints
        SIEsetEndpointStatus(IDX2EP(EP0OUT), SIE_SES_CND_ST);
    } else {
        SIEsetEndpointStatus(bEP, SIE_SES_ST);

        // Update stall state
        endpointStallState |= EP(endpoint);
    }
}

void USBHAL::unstallEndpoint(uint8_t bEP)
{
    uint8_t endpoint = EP2IDX(bEP);

    // Unstall an endpoint. The endpoint will also be reinitialised
    SIEsetEndpointStatus(bEP, 0);

    // Update stall state
    endpointStallState &= ~EP(endpoint);
}

bool USBHAL::getEndpointStallState(uint8_t bEP)
{
    // Returns true if endpoint stalled
    return endpointStallState & EP(EP2IDX(bEP));
}

void USBHAL::remoteWakeup(void) {
    // Remote wakeup
    uint8_t status;

    // Enable USB clocks
    LPC_USB->USBClkCtrl |= DEV_CLK_EN | AHB_CLK_EN;
    while (LPC_USB->USBClkSt != (DEV_CLK_ON | AHB_CLK_ON));

    status = SIEgetDeviceStatus();
    SIEsetDeviceStatus(status & ~SIE_DS_SUS);
}

uint16_t USBHAL::lastFrame(void)
{
    return SIEgetFrameNumber();
}

extern "C" {
    // __attribute__ ((interrupt))
    void USB_IRQHandler(void) {
        ENTER_ISR();
        USBHAL::_usbisr();
        LEAVE_ISR();
    }
}

void USBHAL::_usbisr(void) {
    instance->usbisr();
}

// enable or disable EP interrupt
void USBHAL::endpointSetInterrupt(uint8_t bEP, bool enable)
{
    uint8_t endpoint = EP2IDX(bEP);

    if (enable) {
        enableEndpointInt(bEP);
    } else {
        disableEndpointInt(bEP);
    }
}

bool USBHAL::endpointGetInterrupt(uint8_t bEP)
{
    uint8_t endpoint = EP2IDX(bEP);

    return LPC_USB->USBEpIntEn & EP(endpoint);
}

// trigger EP interrupt
void USBHAL::endpointTriggerInterrupt(uint8_t bEP)
{
    uint8_t endpoint = EP2IDX(bEP);

    LPC_USB->USBEpIntSet = EP(endpoint);
}

void USBHAL::usbisr(void) {
    uint8_t devStat;

    if (LPC_USB->USBDevIntSt & FRAME) {
        // Start of frame event
//         SOF(SIEgetFrameNumber());
        USBEvent_Frame(SIEgetFrameNumber());
        // Clear interrupt status flag
        LPC_USB->USBDevIntClr = FRAME;
    }

    if (LPC_USB->USBDevIntSt & DEV_STAT) {
        // Device Status interrupt
        // Must clear the interrupt status flag before reading the device status from the SIE
        LPC_USB->USBDevIntClr = DEV_STAT;

        // Read device status from SIE
        devStat = SIEgetDeviceStatus();

        if (devStat & SIE_DS_SUS_CH) {
            // Suspend status changed
//             if((devStat & SIE_DS_SUS) != 0) {
//                 USBEvent_suspendStateChanged(false);
//             }
            USBEvent_suspendStateChanged(devStat & SIE_DS_SUS);
        }

        if (devStat & SIE_DS_RST) {
            // Bus reset
//             if((devStat & SIE_DS_SUS) == 0) {
//                 USBEvent_suspendStateChanged(true);
//             }
            USBEvent_busReset();

            realiseEndpoint(IDX2EP(EP0IN), MAX_PACKET_SIZE_EP0, 0);
            realiseEndpoint(IDX2EP(EP0OUT), MAX_PACKET_SIZE_EP0, 0);

            SIEsetMode(SIE_MODE_INAK_CI | SIE_MODE_INAK_CO | SIE_MODE_INAK_BI | SIE_MODE_INAK_BO);
        }

        if (devStat & SIE_DS_CON_CH) {
            USBEvent_connectStateChanged(devStat & SIE_DS_CON);
        }
    }

    if (LPC_USB->USBDevIntSt & EP_SLOW) {
        // (Slow) Endpoint Interrupt

        // Process each endpoint interrupt
        if (LPC_USB->USBEpIntSt & EP(EP0OUT)) {
            uint8_t bEPStat = selectEndpointClearInterrupt(IDX2EP(EP0OUT));
            if (bEPStat & SIE_SE_STP) {
                // this is a setup packet
                EP0setupCallback();
            }
            else if (bEPStat & EPSTAT_FE) { // OUT endpoint, FE = 1 - data in buffer
                EP0out();
            }
        }
        if (LPC_USB->USBEpIntSt & EP(EP0IN)) {
            uint8_t bEPStat = selectEndpointClearInterrupt(IDX2EP(EP0IN));
            //!LPC_USB->USBDevIntClr = EP_SLOW;
            if ((bEPStat & EPSTAT_FE) == 0) // IN endpoint, FE = 0 - empty space in buffer
                EP0in();
        }

        //uint32_t USBEpIntSt = LPC_USB->USBEpIntSt & ~3;
        //if (USBEpIntSt)
        {
            int i;
            uint32_t bitmask;

            for (i = 2, bitmask = 4; i < 32; i++, bitmask <<= 1) {
                uint8_t bEPStat = 255;
                uint8_t ep = IDX2EP(i);
                if (LPC_USB->USBEpIntSt & bitmask) {
                    bEPStat = selectEndpointClearInterrupt(ep);
                    epComplete |= EP(i);
                
                    uint8_t bStat = ((bEPStat & EPSTAT_FE ) ? EP_STATUS_DATA    : 0) |
                                    ((bEPStat & EPSTAT_ST ) ? EP_STATUS_STALLED : 0) |
                                    ((bEPStat & EPSTAT_STP) ? EP_STATUS_SETUP   : 0) |
                                    ((bEPStat & EPSTAT_EPN) ? EP_STATUS_NACKED  : 0) |
                                    ((bEPStat & EPSTAT_PO ) ? EP_STATUS_ERROR   : 0);

                    bool r = true;

                    if (IN_EP(i)) {
//!                        if ((bEPStat & EPSTAT_FE) == 0) // IN endpoint, FE = 0 - empty space in buffer
                            r = USBEvent_EPIn(ep, bStat);
                    } else {
//!                        if (bEPStat & EPSTAT_FE) // OUT endpoint, FE = 1 - data in buffer
                            r = USBEvent_EPOut(ep, bStat);
                    }

                    if (r) {
                      epComplete &= ~EP(i);
                    }
                }
            }
        }

        LPC_USB->USBDevIntClr = EP_SLOW;
    }
}

#endif
