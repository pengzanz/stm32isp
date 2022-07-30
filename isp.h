#ifndef ISP_H
#define ISP_H

#include <QObject>
#include <QSerialPort>

#define ISP_ACK	        0x79
#define ISP_NACK	    0x1F

#define ISP_CMD_INIT	0x7F
#define ISP_CMD_GET	    0x00	/* get the version and command supported */
#define ISP_CMD_GVR	    0x01	/* get version and read protection status */
#define ISP_CMD_GID	    0x02	/* get ID */
#define ISP_CMD_RM	    0x11	/* read memory */
#define ISP_CMD_GO	    0x21	/* go */
#define ISP_CMD_WM	    0x31	/* write memory */
#define ISP_CMD_WM_NS	0x32	/* no-stretch write memory */
#define ISP_CMD_ER	    0x43	/* erase */
#define ISP_CMD_EE	    0x44	/* extended erase */
#define ISP_CMD_EE_NS	0x45	/* extended erase no-stretch */
#define ISP_CMD_WP	    0x63	/* write protect */
#define ISP_CMD_WP_NS	0x64	/* write protect no-stretch */
#define ISP_CMD_UW	    0x73	/* write unprotect */
#define ISP_CMD_UW_NS	0x74	/* write unprotect no-stretch */
#define ISP_CMD_RP	    0x82	/* readout protect */
#define ISP_CMD_RP_NS	0x83	/* readout protect no-stretch */
#define ISP_CMD_UR	    0x92	/* readout unprotect */
#define ISP_CMD_UR_NS	0x93	/* readout unprotect no-stretch */
#define ISP_CMD_CRC	    0xA1	/* compute CRC */
#define ISP_CMD_ERR	    0xFF	/* not a valid command */

class Isp : public QObject
{
    Q_OBJECT
public:
    explicit Isp(QObject *parent = nullptr);
    void set_comName(QString str);
    void set_fileName(QString str);
    int connect(QString comName_);
    int disconnect(void);
    int get_version(void);
    int get_id(void);
    int erase_chip(void);

signals:
    void send_isp_msg(QString str);

private:
    QSerialPort *pSerial;
    QString comName;
    QString fileName;
    bool isConnect;
};

#endif // ISP_H