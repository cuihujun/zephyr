/*
 * Copyright (c) 2016, Intel Corporation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of the Intel Corporation nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE INTEL CORPORATION OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __QM_UART_H__
#define __QM_UART_H__

#include "qm_common.h"
#include "qm_soc_regs.h"
#include "qm_dma.h"

/**
 * UART peripheral driver.
 *
 * @defgroup groupUART UART
 * @{
 */

/** Divisor Latch Access Bit. */
#define QM_UART_LCR_DLAB BIT(7)
/** Auto Flow Control Enable Bit. */
#define QM_UART_MCR_AFCE BIT(5)
/** Request to Send Bit. */
#define QM_UART_MCR_RTS BIT(1)

/** FIFO Enable Bit. */
#define QM_UART_FCR_FIFOE BIT(0)
/** Reset Receive FIFO. */
#define QM_UART_FCR_RFIFOR BIT(1)
/** Reset Transmit FIFO. */
#define QM_UART_FCR_XFIFOR BIT(2)
/** FIFO half RX, half TX Threshold. */
#define QM_UART_FCR_DEFAULT_TX_RX_THRESHOLD (0xB0)
/** FIFO 1 byte RX, half TX Threshold. */
#define QM_UART_FCR_TX_1_2_RX_0_THRESHOLD (0x30)
/** FIFO half RX, empty TX Threshold. */
#define QM_UART_FCR_TX_0_RX_1_2_THRESHOLD (0x80)

/** Transmit Holding Register Empty. */
#define QM_UART_IIR_THR_EMPTY (0x02)
/** Received Data Available. */
#define QM_UART_IIR_RECV_DATA_AVAIL (0x04)
/** Receiver Line Status. */
#define QM_UART_IIR_RECV_LINE_STATUS (0x06)
/** Character Timeout. */
#define QM_UART_IIR_CHAR_TIMEOUT (0x0C)
/** Interrupt ID Mask. */
#define QM_UART_IIR_IID_MASK (0x0F)

/** Data Ready Bit. */
#define QM_UART_LSR_DR BIT(0)
/** Overflow Error Bit. */
#define QM_UART_LSR_OE BIT(1)
/** Parity Error Bit. */
#define QM_UART_LSR_PE BIT(2)
/** Framing Error Bit. */
#define QM_UART_LSR_FE BIT(3)
/** Break Interrupt Bit. */
#define QM_UART_LSR_BI BIT(4)
/** Transmit Holding Register Empty Bit. */
#define QM_UART_LSR_THRE BIT(5)
/** Transmitter Empty Bit. */
#define QM_UART_LSR_TEMT BIT(6)
/** Receiver FIFO Error Bit. */
#define QM_UART_LSR_RFE BIT(7)

/** Enable Received Data Available Interrupt. */
#define QM_UART_IER_ERBFI BIT(0)
/** Enable Transmit Holding Register Empty Interrupt. */
#define QM_UART_IER_ETBEI BIT(1)
/** Enable Receiver Line Status Interrupt. */
#define QM_UART_IER_ELSI BIT(2)
/** Programmable THRE Interrupt Mode. */
#define QM_UART_IER_PTIME BIT(7)

/** Line Status Errors. */
#define QM_UART_LSR_ERROR_BITS                                                 \
	(QM_UART_LSR_OE | QM_UART_LSR_PE | QM_UART_LSR_FE | QM_UART_LSR_BI)

/** FIFO Depth. */
#define QM_UART_FIFO_DEPTH (16)
/** FIFO Half Depth. */
#define QM_UART_FIFO_HALF_DEPTH (QM_UART_FIFO_DEPTH / 2)

/** Divisor Latch High Offset. */
#define QM_UART_CFG_BAUD_DLH_OFFS 16
/** Divisor Latch Low Offset. */
#define QM_UART_CFG_BAUD_DLL_OFFS 8
/** Divisor Latch Fraction Offset. */
#define QM_UART_CFG_BAUD_DLF_OFFS 0
/** Divisor Latch High Mask. */
#define QM_UART_CFG_BAUD_DLH_MASK (0xFF << QM_UART_CFG_BAUD_DLH_OFFS)
/** Divisor Latch Low Mask. */
#define QM_UART_CFG_BAUD_DLL_MASK (0xFF << QM_UART_CFG_BAUD_DLL_OFFS)
/** Divisor Latch Fraction Mask. */
#define QM_UART_CFG_BAUD_DLF_MASK (0xFF << QM_UART_CFG_BAUD_DLF_OFFS)

/** Divisor Latch Packing Helper. */
#define QM_UART_CFG_BAUD_DL_PACK(dlh, dll, dlf)                                \
	(dlh << QM_UART_CFG_BAUD_DLH_OFFS | dll << QM_UART_CFG_BAUD_DLL_OFFS | \
	 dlf << QM_UART_CFG_BAUD_DLF_OFFS)

/** Divisor Latch High Unpacking Helper. */
#define QM_UART_CFG_BAUD_DLH_UNPACK(packed)                                    \
	((packed & QM_UART_CFG_BAUD_DLH_MASK) >> QM_UART_CFG_BAUD_DLH_OFFS)
/** Divisor Latch Low Unpacking Helper. */
#define QM_UART_CFG_BAUD_DLL_UNPACK(packed)                                    \
	((packed & QM_UART_CFG_BAUD_DLL_MASK) >> QM_UART_CFG_BAUD_DLL_OFFS)
/** Divisor Latch Fraction Unpacking Helper. */
#define QM_UART_CFG_BAUD_DLF_UNPACK(packed)                                    \
	((packed & QM_UART_CFG_BAUD_DLF_MASK) >> QM_UART_CFG_BAUD_DLF_OFFS)

/**
 * UART Line control.
 */
typedef enum {
	QM_UART_LC_5N1 = 0x00,   /**< 5 data bits, no parity, 1 stop bit. */
	QM_UART_LC_5N1_5 = 0x04, /**< 5 data bits, no parity, 1.5 stop bits. */
	QM_UART_LC_5E1 = 0x18,   /**< 5 data bits, even parity, 1 stop bit. */
	QM_UART_LC_5E1_5 = 0x1c, /**< 5 data bits, even par., 1.5 stop bits. */
	QM_UART_LC_5O1 = 0x08,   /**< 5 data bits, odd parity, 1 stop bit. */
	QM_UART_LC_5O1_5 = 0x0c, /**< 5 data bits, odd parity, 1.5 stop bits. */
	QM_UART_LC_6N1 = 0x01,   /**< 6 data bits, no parity, 1 stop bit. */
	QM_UART_LC_6N2 = 0x05,   /**< 6 data bits, no parity, 2 stop bits. */
	QM_UART_LC_6E1 = 0x19,   /**< 6 data bits, even parity, 1 stop bit. */
	QM_UART_LC_6E2 = 0x1d,   /**< 6 data bits, even parity, 2 stop bits. */
	QM_UART_LC_6O1 = 0x09,   /**< 6 data bits, odd parity, 1 stop bit. */
	QM_UART_LC_6O2 = 0x0d,   /**< 6 data bits, odd parity, 2 stop bits. */
	QM_UART_LC_7N1 = 0x02,   /**< 7 data bits, no parity, 1 stop bit. */
	QM_UART_LC_7N2 = 0x06,   /**< 7 data bits, no parity, 2 stop bits. */
	QM_UART_LC_7E1 = 0x1a,   /**< 7 data bits, even parity, 1 stop bit. */
	QM_UART_LC_7E2 = 0x1e,   /**< 7 data bits, even parity, 2 stop bits. */
	QM_UART_LC_7O1 = 0x0a,   /**< 7 data bits, odd parity, 1 stop bit. */
	QM_UART_LC_7O2 = 0x0e,   /**< 7 data bits, odd parity, 2 stop bits. */
	QM_UART_LC_8N1 = 0x03,   /**< 8 data bits, no parity, 1 stop bit. */
	QM_UART_LC_8N2 = 0x07,   /**< 8 data bits, no parity, 2 stop bits. */
	QM_UART_LC_8E1 = 0x1b,   /**< 8 data bits, even parity, 1 stop bit. */
	QM_UART_LC_8E2 = 0x1f,   /**< 8 data bits, even parity, 2 stop bits. */
	QM_UART_LC_8O1 = 0x0b,   /**< 8 data bits, odd parity, 1 stop bit. */
	QM_UART_LC_8O2 = 0x0f    /**< 8 data bits, odd parity, 2 stop bits. */
} qm_uart_lc_t;

/**
 * UART Status type.
 */
typedef enum {
	QM_UART_IDLE = 0,	   /**< IDLE. */
	QM_UART_RX_OE = BIT(1),     /**< Receiver overrun. */
	QM_UART_RX_PE = BIT(2),     /**< Parity error. */
	QM_UART_RX_FE = BIT(3),     /**< Framing error. */
	QM_UART_RX_BI = BIT(4),     /**< Break interrupt. */
	QM_UART_TX_BUSY = BIT(5),   /**< TX Busy flag. */
	QM_UART_RX_BUSY = BIT(6),   /**< RX Busy flag. */
	QM_UART_TX_NFULL = BIT(7),  /**< TX FIFO not full. */
	QM_UART_RX_NEMPTY = BIT(8), /**< RX FIFO not empty. */
} qm_uart_status_t;

/**
 * UART configuration type.
 */
typedef struct {
	qm_uart_lc_t line_control; /**< Line control (enum). */
	uint32_t baud_divisor;     /**< Baud Divisor. */
	bool hw_fc;		   /**< Hardware Automatic Flow Control. */
	bool int_en;		   /**< Interrupt enable. */
} qm_uart_config_t;

/**
 * UART asynchronous transfer structure.
 */
typedef struct {
	uint8_t *data;     /**< Pre-allocated write or read buffer. */
	uint32_t data_len; /**< Number of bytes to transfer. */

	/** Transfer callback
	 *
	 * @param[in] data Callback user data.
	 * @param[in] error 0 on success.
	 *                  Negative @ref errno for possible error codes.
	 * @param[in] status UART module status
	 * @param[in] len Length of the UART transfer if successful, 0
	 * otherwise.
	 */
	void (*callback)(void *data, int error, qm_uart_status_t status,
			 uint32_t len);
	void *callback_data; /**< Callback identifier. */
} qm_uart_transfer_t;

/**
 * Set UART configuration.
 *
 * Change the configuration of a UART module. This includes line control,
 * baud rate and hardware flow control.
 *
 * @param[in] uart Which UART module to configure.
 * @param[in] cfg New configuration for UART. This must not be NULL.
 *
 * @return Standard errno return type for QMSI.
 * @retval 0 on success.
 * @retval Negative @ref errno for possible error codes.
 */
int qm_uart_set_config(const qm_uart_t uart, const qm_uart_config_t *const cfg);

/**
 * Get UART bus status.
 *
 * Retrieve UART interface status. Return QM_UART_BUSY if transmitting
 * data; QM_UART_IDLE if available for transfer QM_UART_TX_ERROR if an
 * error has occurred in transmission.
 *
 * @param[in] uart Which UART to read the status of.
 * @param[out] status UART specific status. This must not be NULL.
 *
 * @return Standard errno return type for QMSI.
 * @retval 0 on success.
 * @retval Negative @ref errno for possible error codes.
 */
int qm_uart_get_status(const qm_uart_t uart, qm_uart_status_t *const status);

/**
 * UART character data write.
 *
 * Perform a single character write on the UART interface.
 * This is a blocking synchronous call.
 *
 * @param[in] uart UART index.
 * @param[in] data Data to write to UART.
 *
 * @return Standard errno return type for QMSI.
 * @retval 0 on success.
 * @retval Negative @ref errno for possible error codes.
 */
int qm_uart_write(const qm_uart_t uart, const uint8_t data);

/**
 * UART character data read.
 *
 * Perform a single character read from the UART interface.
 * This is a blocking synchronous call.
 *
 * @param[in] uart UART index.
 * @param[out] data Data to read from UART. This must not be NULL.
 * @param[out] status UART specific status.
 *
 * @return Standard errno return type for QMSI.
 * @retval 0 on success.
 * @retval Negative @ref errno for possible error codes.
 */
int qm_uart_read(const qm_uart_t uart, uint8_t *const data,
		 qm_uart_status_t *const status);

/**
 * UART character data write.
 *
 * Perform a single character write on the UART interface.
 * This is a non-blocking synchronous call.
 *
 * @param[in] uart UART index.
 * @param[in] data Data to write to UART.
 *
 * @return Standard errno return type for QMSI.
 * @retval 0 on success.
 * @retval Negative @ref errno for possible error codes.
 */
int qm_uart_write_non_block(const qm_uart_t uart, const uint8_t data);

/**
 * UART character data read.
 *
 * Perform a single character read from the UART interface.
 * This is a non-blocking synchronous call.
 *
 * @param[in] uart UART index.
 * @param[out] data Character read. This must not be NULL.
 *
 * @return Standard errno return type for QMSI.
 * @retval 0 on success.
 * @retval Negative @ref errno for possible error codes.
 */
int qm_uart_read_non_block(const qm_uart_t uart, uint8_t *const data);

/**
 * UART multi-byte data write.
 *
 * Perform a write on the UART interface. This is a blocking
 * synchronous call. The function will block until all data has
 * been transferred.
 *
 * @param[in] uart UART index.
 * @param[in] data Data to write to UART. This must not be NULL.
 * @param[in] len Length of data to write to UART.
 *
 * @return Standard errno return type for QMSI.
 * @retval 0 on success.
 * @retval Negative @ref errno for possible error codes.
 */
int qm_uart_write_buffer(const qm_uart_t uart, const uint8_t *const data,
			 uint32_t len);

/**
 * Interrupt based TX on UART.
 *
 * Perform an interrupt based TX transfer on the UART bus. The function
 * will replenish the TX FIFOs on UART empty interrupts.
 *
 * @param[in] uart UART index.
 * @param[in] xfer Structure containing pre-allocated
 *                 write buffer and callback functions.
 *                 This must not be NULL.
 *
 * @return Standard errno return type for QMSI.
 * @retval 0 on success.
 * @retval Negative @ref errno for possible error codes.
 */
int qm_uart_irq_write(const qm_uart_t uart,
		      const qm_uart_transfer_t *const xfer);

/**
 * Interrupt based RX on UART.
 *
 * Perform an interrupt based RX transfer on the UART bus. The function
 * will read back the RX FIFOs on UART empty interrupts.
 *
 * @param[in] uart UART index.
 * @param[in] xfer Structure containing pre-allocated read
 *                 buffer and callback functions.
 *                 This must not be NULL.
 *
 * @return Standard errno return type for QMSI.
 * @retval 0 on success.
 * @retval Negative @ref errno for possible error codes.
 */
int qm_uart_irq_read(const qm_uart_t uart,
		     const qm_uart_transfer_t *const xfer);

/**
 * Terminate UART IRQ TX transfer.
 *
 * Terminate the current IRQ TX transfer on the UART bus.
 * This will cause the relevant callbacks to be called.
 *
 * @param[in] uart UART index.
 *
 * @return Standard errno return type for QMSI.
 * @retval 0 on success.
 * @retval Negative @ref errno for possible error codes.
 */
int qm_uart_irq_write_terminate(const qm_uart_t uart);

/**
 * Terminate UART IRQ RX transfer.
 *
 * Terminate the current IRQ RX transfer on the UART bus.
 * This will cause the relevant callbacks to be called.
 *
 * @param[in] uart UART index.
 *
 * @return Standard errno return type for QMSI.
 * @retval 0 on success.
 * @retval Negative @ref errno for possible error codes.
 */
int qm_uart_irq_read_terminate(const qm_uart_t uart);

/**
 * Configure a DMA channel with a specific transfer direction.
 *
 * The user is responsible for managing the allocation of the pool
 * of DMA channels provided by each DMA core to the different
 * peripheral drivers that require them.
 *
 * This function configures DMA channel parameters that are unlikely to change
 * between transfers, like transaction width, burst size, and handshake
 * interface parameters. The user will likely only call this function once for
 * the lifetime of an application unless the channel needs to be repurposed.
 *
 * Note that qm_dma_init() must first be called before configuring a channel.
 *
 * @param[in] uart UART index.
 * @param[in] dma_ctrl_id DMA controller identifier.
 * @param[in] dma_channel_id DMA channel identifier.
 * @param[in] dma_channel_direction DMA channel direction, either
 * QM_DMA_MEMORY_TO_PERIPHERAL (write transfer) or QM_DMA_PERIPHERAL_TO_MEMORY
 * (read transfer).
 *
 * @return Standard errno return type for QMSI.
 * @retval 0 on success.
 * @retval Negative @ref errno for possible error codes.
 */
int qm_uart_dma_channel_config(
    const qm_uart_t uart, const qm_dma_t dma_ctrl_id,
    const qm_dma_channel_id_t dma_channel_id,
    const qm_dma_channel_direction_t dma_channel_direction);

/**
 * Perform a DMA-based TX transfer on the UART bus.
 *
 * In order for this call to succeed, previously the user
 * must have configured a DMA channel with direction
 * QM_DMA_MEMORY_TO_PERIPHERAL to be used on this UART, calling
 * qm_uart_dma_channel_config(). The transfer length is limited to 4KB.
 *
 * @param[in] uart UART index.
 * @param[in] xfer Structure containing a pre-allocated write buffer
 *                 and callback functions.
 *                 This must not be NULL.
 *                 Callback pointer must not be NULL.
 *
 * @return Standard errno return type for QMSI.
 * @retval 0 on success.
 * @retval Negative @ref errno for possible error codes.
 */
int qm_uart_dma_write(const qm_uart_t uart,
		      const qm_uart_transfer_t *const xfer);

/**
 * Perform a DMA-based RX transfer on the UART bus.
 *
 * In order for this call to succeed, previously the user
 * must have configured a DMA channel with direction
 * QM_DMA_PERIPHERAL_TO_MEMORY to be used on this UART, calling
 * qm_uart_dma_channel_config(). The transfer length is limited to 4KB.
 *
 * @param[in] uart UART index.
 * @param[in] xfer Structure containing a pre-allocated read buffer
 *                 and callback functions.
 *                 This must not be NULL.
 *                 Callback pointer must not be NULL.
 *
 * @return Standard errno return type for QMSI.
 * @retval 0 on success.
 * @retval Negative @ref errno for possible error codes.
 */
int qm_uart_dma_read(const qm_uart_t uart,
		     const qm_uart_transfer_t *const xfer);

/**
 * Terminate the current DMA TX transfer on the UART bus.
 *
 * This will cause the relevant callbacks to be called.
 *
 * @param[in] uart UART index.
 *
 * @return Standard errno return type for QMSI.
 * @retval 0 on success.
 * @retval Negative @ref errno for possible error codes.
 */
int qm_uart_dma_write_terminate(const qm_uart_t uart);

/**
 * Terminate the current DMA RX transfer on the UART bus.
 *
 * This will cause the relevant callbacks to be called.
 *
 * @param[in] uart UART index.
 *
 * @return Standard errno return type for QMSI.
 * @retval 0 on success.
 * @retval Negative @ref errno for possible error codes.
 */
int qm_uart_dma_read_terminate(const qm_uart_t uart);

/**
 * @}
 */

#endif /* __QM_UART_H__ */
