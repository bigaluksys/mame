// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

    Skeleton driver for Philips VP415 LV ROM Player

	List of Modules:
		A - Audio Processor
		B - RGB
		C - Video Processor
		D - Ref Source
		E - Slide Drive
		F - Motor+Sequence
		G - Gen Lock
		H - ETBC B
		I - ETBC C
		J - Focus
		K - HF Processor
		L - Video Dropout Correction
		M - Radial
		N - Display Keyboard
		P - Front Loader
		Q - RC5 Mirror
		R - Drive Processor
		S - Control
		T - Supply
		U - Analog I/O
		V - Module Carrier
		W - CPU Datagrabber
		X - LV ROM
		Y - Vid Mix
		Z - Deck Electronics

	TODO:
	- Module W has a NEC D8041AHC which has not yet been dumped. A dump
	  will be necessary to properly emulate the player.
	- Driver currently fails the initial self-test with code 053. Per
	  the service manual, code 053 means "no lead-in code at start-up of
	  player (diagnostics)".

***************************************************************************/

#include "emu.h"
#include "screen.h"

#include "cpu/mcs51/mcs51.h"
#include "cpu/mcs48/mcs48.h"
#include "cpu/z80/z80.h"

#include "machine/i8155.h"
#include "machine/i8255.h"
#include "machine/ncr5385.h"
#include "machine/saa1043.h"
#include "video/mb88303.h"

class vp415_state : public driver_device
{
public:
	vp415_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_datacpu(*this, Z80CPU_TAG)
		, m_scsi(*this, SCSI_TAG)
		, m_drivecpu(*this, DRIVECPU_TAG)
		, m_ctrlcpu(*this, CTRLCPU_TAG)
		, m_ctrlmcu(*this, CTRLMCU_TAG)
		, m_chargen(*this, CHARGEN_TAG)
		, m_mainram(*this, Z80RAM_TAG)
		, m_ctrlram(*this, CTRLRAM_TAG)
		, m_switches(*this, SWITCHES_TAG)
	{ }

	void vp415(machine_config &config);

	virtual void machine_reset() override;
	virtual void machine_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	DECLARE_WRITE_LINE_MEMBER(cpu_int1_w);

	DECLARE_WRITE8_MEMBER(sel34_w);
	DECLARE_READ8_MEMBER(sel37_r);

	DECLARE_WRITE8_MEMBER(ctrl_regs_w);
	DECLARE_READ8_MEMBER(ctrl_regs_r);
	DECLARE_WRITE8_MEMBER(ctrl_cpu_port1_w);
	DECLARE_READ8_MEMBER(ctrl_cpu_port1_r);
	DECLARE_WRITE8_MEMBER(ctrl_cpu_port3_w);
	DECLARE_READ8_MEMBER(ctrl_cpu_port3_r);

	DECLARE_WRITE8_MEMBER(ctrl_mcu_port1_w);
	DECLARE_READ8_MEMBER(ctrl_mcu_port1_r);
	DECLARE_WRITE8_MEMBER(ctrl_mcu_port2_w);
	DECLARE_READ8_MEMBER(ctrl_mcu_port2_r);

	DECLARE_READ8_MEMBER(drive_i8155_pb_r);
	DECLARE_READ8_MEMBER(drive_i8155_pc_r);

	DECLARE_WRITE8_MEMBER(drive_i8255_pa_w);
	DECLARE_WRITE8_MEMBER(drive_i8255_pb_w);
	DECLARE_READ8_MEMBER(drive_i8255_pc_r);
	DECLARE_WRITE8_MEMBER(drive_cpu_port1_w);
	DECLARE_WRITE8_MEMBER(drive_cpu_port3_w);

	DECLARE_WRITE_LINE_MEMBER(refv_w);

	static const char* Z80CPU_TAG;
	static const char* Z80RAM_TAG;
	static const char* SCSI_TAG;
	static const char* CTRLCPU_TAG;
	static const char* CTRLMCU_TAG;
	static const char* CTRLRAM_TAG;
	static const char* DRIVECPU_TAG;
	static const char* DESCRAMBLE_ROM_TAG;
	static const char* SYNC_ROM_TAG;
	static const char* DRIVE_ROM_TAG;
	static const char* CONTROL_ROM_TAG;
	static const char* SWITCHES_TAG;
	static const char* I8155_TAG;
	static const char* I8255_TAG;
	static const char* CHARGEN_TAG;
	static const char* SYNCGEN_TAG;

	static const device_timer_id DRIVE_2PPR_ID;

protected:
	// CPU Board enums
	enum
	{
		SEL34_INTR_N = 0x01,
		SEL34_RES    = 0x20,
		SEL34_ERD    = 0x40,
		SEL34_ENW    = 0x80,
		SEL34_INTR_N_BIT = 0,
		SEL34_RES_BIT    = 5,
		SEL34_ERD_BIT    = 6,
		SEL34_ENW_BIT    = 7,
	};

	enum
	{
		SEL37_ID0   = 0x01,
		SEL37_ID1   = 0x02,
		SEL37_BRD   = 0x10,
		SEL37_MON_N = 0x20,
		SEL37_SK1c  = 0x40,
		SEL37_SK1d  = 0x40,

		SEL37_ID0_BIT   = 0,
		SEL37_ID1_BIT   = 1,
		SEL37_BRD_BIT   = 4,
		SEL37_MON_N_BIT = 5,
		SEL37_SK1c_BIT  = 6,
		SEL37_SK1d_BIT  = 7,
	};

	// Control Board enums
	enum
	{
		CTRL_P3_INT1 = 0x08,

		CTRL_P3_INT1_BIT = 3
	};

	// Drive Board enums
	enum
	{
		I8255PC_NOT_FOCUSED     = 0x02,
		I8255PC_0RPM_N          = 0x08,
		I8255PC_DISC_REFLECTION = 0x10,
	};

	enum
	{
		I8255PB_COMM1    = 0x01,
		I8255PB_COMM2    = 0x02,
		I8255PB_COMM3    = 0x04,
		I8255PB_COMM4    = 0x08,
		I8255PB_RLS_N    = 0x10,
		I8255PB_SL_PWR   = 0x20,
		I8255PB_RAD_FS_N = 0x40,
		I8255PB_STR1     = 0x80,

		I8255PB_COMM1_BIT    = 0,
		I8255PB_COMM2_BIT    = 1,
		I8255PB_COMM3_BIT    = 2,
		I8255PB_COMM4_BIT    = 3,
		I8255PB_RLS_N_BIT    = 4,
		I8255PB_SL_PWR_BIT   = 5,
		I8255PB_RAD_FS_N_BIT = 6,
		I8255PB_STR1_BIT     = 7,
	};

	enum
	{
		I8155PB_2PPR    = 0x01,
		I8155PB_RAD_MIR = 0x04,
		I8155PB_FRLOCK  = 0x08,

		I8155PB_2PPR_BIT    = 0,
		I8155PB_RAD_MIR_BIT = 2,
		I8155PB_FRLOCK_BIT  = 3,
	};

	enum
	{
		DRIVE_P1_CP1    = 0x01,
		DRIVE_P1_CP2    = 0x02,
		DRIVE_P1_LDI    = 0x04,
		DRIVE_P1_ATN_N  = 0x08,
		DRIVE_P1_TX     = 0x10,
		DRIVE_P1_STB_N  = 0x20,
		DRIVE_P1_STR0_N = 0x40,
		DRIVE_P1_TP2    = 0x80,

		DRIVE_P1_CP1_BIT    = 0,
		DRIVE_P1_CP2_BIT    = 1,
		DRIVE_P1_LDI_BIT    = 2,
		DRIVE_P1_ATN_N_BIT  = 3,
		DRIVE_P1_TX_BIT     = 4,
		DRIVE_P1_STB_N_BIT  = 5,
		DRIVE_P1_STR0_N_BIT = 6,
		DRIVE_P1_TP2_BIT    = 7
	};

	virtual void video_start() override;

	void z80_program_map(address_map &map);
	void z80_io_map(address_map &map);
	void set_int_line(uint8_t line, uint8_t value);
	void update_cpu_int();

	void ctrl_program_map(address_map &map);
	void ctrl_io_map(address_map &map);
	void ctrlmcu_program_map(address_map &map);
	void sd_w(uint8_t data);
	uint8_t sd_r();

	void drive_program_map(address_map &map);
	void drive_io_map(address_map &map);

	required_device<z80_device> m_datacpu;
	required_device<ncr5385_device> m_scsi;
	required_device<i8031_device> m_drivecpu;
	required_device<i8031_device> m_ctrlcpu;
	required_device<i8041_device> m_ctrlmcu;
	required_device<mb88303_device> m_chargen;
	required_shared_ptr<uint8_t> m_mainram;
	required_shared_ptr<uint8_t> m_ctrlram;
	required_ioport m_switches;

	uint8_t m_sel34;
	uint8_t m_sel37;

	uint8_t m_int_lines[2];

	uint8_t m_refv;

	uint8_t m_ctrl_cpu_p1;
	uint8_t m_ctrl_cpu_p3;
	uint8_t m_ctrl_mcu_p1;
	uint8_t m_ctrl_mcu_p2;

	uint8_t m_drive_p1;
	uint8_t m_drive_pc_bits;

	uint8_t m_drive_rad_mir_dac;
	uint8_t m_drive_i8255_pb;
	emu_timer *m_drive_2ppr_timer;
	uint8_t m_drive_2ppr;
};

/*static*/ const char* vp415_state::Z80CPU_TAG = "z80cpu";
/*static*/ const char* vp415_state::Z80RAM_TAG = "z80ram";
/*static*/ const char* vp415_state::SCSI_TAG = "ncr5385";
/*static*/ const char* vp415_state::CTRLCPU_TAG = "ctrlcpu";
/*static*/ const char* vp415_state::CTRLMCU_TAG = "ctrlmcu";
/*static*/ const char* vp415_state::CTRLRAM_TAG = "ctrlram";
/*static*/ const char* vp415_state::DRIVECPU_TAG = "drivecpu";
/*static*/ const char* vp415_state::DESCRAMBLE_ROM_TAG = "descramblerom";
/*static*/ const char* vp415_state::SYNC_ROM_TAG = "syncrom";
/*static*/ const char* vp415_state::DRIVE_ROM_TAG = "driverom";
/*static*/ const char* vp415_state::CONTROL_ROM_TAG = "controlrom";
/*static*/ const char* vp415_state::SWITCHES_TAG = "SWITCHES";
/*static*/ const char* vp415_state::I8155_TAG = "i8155";
/*static*/ const char* vp415_state::I8255_TAG = "i8255";
/*static*/ const char* vp415_state::CHARGEN_TAG = "mb88303";
/*static*/ const char* vp415_state::SYNCGEN_TAG = "saa1043";

/*static*/ const device_timer_id vp415_state::DRIVE_2PPR_ID = 0;

void vp415_state::machine_reset()
{
	m_sel34 = 0;
	m_sel37 = SEL37_BRD | SEL37_MON_N | SEL37_SK1c | SEL37_SK1d;
	m_int_lines[0] = 0;
	m_int_lines[1] = 0;

	m_ctrl_cpu_p1 = 0;
	m_ctrl_cpu_p3 = 0;

	m_ctrl_mcu_p1 = 0;
	m_ctrl_mcu_p2 = 0;

	m_drive_p1 = 0;
	m_drive_i8255_pb = 0;

	m_drive_pc_bits = I8255PC_DISC_REFLECTION | I8255PC_NOT_FOCUSED;

	m_drive_rad_mir_dac = 0;

	m_drive_2ppr = 0;
	m_drive_2ppr_timer = timer_alloc(DRIVE_2PPR_ID);
	m_drive_2ppr_timer->adjust(attotime::from_msec(10));
}

void vp415_state::machine_start()
{
}

void vp415_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
		case DRIVE_2PPR_ID:
			m_drive_2ppr ^= I8155PB_2PPR;
			m_drive_2ppr_timer->adjust(attotime::from_msec(10));
			break;
		default:
			break;
	}
}

WRITE_LINE_MEMBER(vp415_state::refv_w)
{
	m_refv = state;
	m_drivecpu->set_input_line(MCS51_INT0_LINE, m_refv ? CLEAR_LINE : ASSERT_LINE);
	//printf("Current time in ms: %f\n", machine().scheduler().time().as_double() * 1000.0D);
}

// CPU Datagrabber Module (W)

WRITE_LINE_MEMBER(vp415_state::cpu_int1_w)
{
	set_int_line(0, state);
}

WRITE8_MEMBER(vp415_state::sel34_w)
{
	logerror("%s: sel34: /INTR=%d, RES=%d, ERD=%d, ENW=%d\n", machine().describe_context(), BIT(data, SEL34_INTR_N_BIT), BIT(data, SEL34_RES_BIT), BIT(data, SEL34_ERD_BIT), BIT(data, SEL34_ENW_BIT));
	m_sel34 = data;

	if (!BIT(data, SEL34_INTR_N_BIT))
	{
		m_sel37 &= ~(SEL37_ID0 | SEL37_ID1);
		update_cpu_int();
	}
}

READ8_MEMBER(vp415_state::sel37_r)
{
	logerror("%s: sel37: ID0=%d, ID1=%d\n", machine().describe_context(), BIT(m_sel37, SEL37_ID0_BIT), BIT(m_sel37, SEL37_ID1_BIT));
	return m_sel37;
}

void vp415_state::set_int_line(uint8_t line, uint8_t value)
{
	if (value)
	{
		m_sel37 |= line ? SEL37_ID1 : SEL37_ID0;
	}

	update_cpu_int();
}

void vp415_state::update_cpu_int()
{
	m_datacpu->set_input_line(0, (m_sel37 & (SEL37_ID0 | SEL37_ID1)) ? ASSERT_LINE : CLEAR_LINE);
}

void vp415_state::z80_program_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region(Z80CPU_TAG, 0);
	map(0xa000, 0xfeff).ram().share(Z80RAM_TAG);
}

void vp415_state::z80_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x0f).rw(SCSI_TAG, FUNC(ncr5385_device::read), FUNC(ncr5385_device::write));
	// 0x20, 0x21: Connected to A0 + D0..D7 of SLAVE i8041
	map(0x34, 0x34).w(this, FUNC(vp415_state::sel34_w));
	map(0x37, 0x37).r(this, FUNC(vp415_state::sel37_r));
}



// Control Module (S)

WRITE8_MEMBER(vp415_state::ctrl_regs_w)
{
	const uint8_t handler_index = (offset & 0x0c00) >> 10;
	switch (handler_index)
	{
		case 0: // WREN
			logerror("%s: ctrl_regs_w: WREN: %02x\n", machine().describe_context(), data);
			sd_w(data);
			break;
		case 1: // WR3
			logerror("%s: ctrl_regs_w: WR3 (UPI-41): %d=%02x\n", machine().describe_context(), (offset >> 9) & 1, data);
			m_ctrlmcu->upi41_master_w(space, (offset >> 9) & 1, data);
			break;
		case 2:
			logerror("%s: ctrl_regs_w: N.C. write %02x\n", machine().describe_context(), data);
			break;
		case 3:
			logerror("%s: ctrl_regs_w: output buffer: VP=%d, NPL=%d, WR-CLK=%d, DB/STAT=%d, RD-STRT=%d, V/C-TXT=%d\n", machine().describe_context(), data & 0x7, BIT(data, 4), BIT(data, 3), BIT(data, 5), BIT(data, 6), BIT(data, 7));
			break;
	}
}

READ8_MEMBER(vp415_state::ctrl_regs_r)
{
	const uint8_t handler_index = (offset & 0x0c00) >> 10;
	uint8_t value = 0;
	switch (handler_index)
	{
		case 0: // RDEN
			value = sd_r();
			logerror("%s: ctrl_regs_r: RDEN: %02x\n", machine().describe_context(), value);
			break;
		case 1: // /RD3
			value = m_ctrlmcu->upi41_master_r(space, (offset >> 9) & 1);
			logerror("%s: ctrl_regs_r: RD3 (UPI-41): %d (%02x)\n", machine().describe_context(), (offset >> 9) & 1, value);
			break;
		case 2: // /RD2
			logerror("%s: ctrl_regs_r: N.C. read\n", machine().describe_context());
			break;
		case 3:
			value = m_switches->read();
			logerror("%s: ctrl_regs_r: RD1 (DIP switches): %02x\n", machine().describe_context(), value);
			break;
	}
	return value;
}

WRITE8_MEMBER(vp415_state::ctrl_cpu_port1_w)
{
	uint8_t old = m_ctrl_cpu_p1;
	m_ctrl_cpu_p1 = data;

	if ((m_ctrl_cpu_p1 ^ old) & 0xdf) // Ignore petting the watchdog (bit 5)
	{
		logerror("%s: ctrl_cpu_port1_w: %02x\n", machine().describe_context(), data);
	}
}

READ8_MEMBER(vp415_state::ctrl_cpu_port1_r)
{
	uint8_t ret = m_ctrl_cpu_p1;
	m_ctrl_cpu_p1 ^= 0x10;
	logerror("%s: ctrl_cpu_port1_r (%02x)\n", machine().describe_context(), ret);
	return ret;
}

WRITE8_MEMBER(vp415_state::ctrl_cpu_port3_w)
{
	m_ctrl_cpu_p3 = ~data;
	logerror("%s: ctrl_cpu_port3_w: %02x\n", machine().describe_context(), data);
}

READ8_MEMBER(vp415_state::ctrl_cpu_port3_r)
{
	uint8_t ret = m_ctrl_cpu_p3;
	logerror("%s: ctrl_cpu_port3_r (%02x)\n", machine().describe_context(), ret);
	return ret;
}

WRITE8_MEMBER(vp415_state::ctrl_mcu_port1_w)
{
	m_ctrl_mcu_p1 = data;
	logerror("%s: ctrl_mcu_port1_w: %02x\n", machine().describe_context(), data);
}

READ8_MEMBER(vp415_state::ctrl_mcu_port1_r)
{
	uint8_t value = m_ctrl_mcu_p1;
	logerror("%s: ctrl_mcu_port1_r: %02x\n", machine().describe_context(), value);
	return value;
}

WRITE8_MEMBER(vp415_state::ctrl_mcu_port2_w)
{
	m_ctrl_mcu_p2 = data;
	if (BIT(data, 4))
		m_ctrl_cpu_p3 &= CTRL_P3_INT1;
	else
		m_ctrl_cpu_p3 |= CTRL_P3_INT1;
	logerror("%s: ctrl_mcu_port2_w: %02x\n", machine().describe_context(), data);
}

READ8_MEMBER(vp415_state::ctrl_mcu_port2_r)
{
	logerror("%s: ctrl_mcu_port2_r: %02x\n", machine().describe_context(), 0);
	return 0;
}

uint8_t vp415_state::sd_r()
{
	return 0;
}

void vp415_state::sd_w(uint8_t data)
{
}

void vp415_state::ctrl_program_map(address_map &map)
{
	map(0x0000, 0xffff).rom().region(CONTROL_ROM_TAG, 0);
}

void vp415_state::ctrl_io_map(address_map &map)
{
	map(0x0000, 0x1fff).ram().share(CTRLRAM_TAG);
	map(0xe000, 0xffff).rw(this, FUNC(vp415_state::ctrl_regs_r), FUNC(vp415_state::ctrl_regs_w)).mask(0x1e00);
}

void vp415_state::ctrlmcu_program_map(address_map &map)
{
	map(0x0000, 0x03ff).rom().region(CTRLMCU_TAG, 0);
}

// Drive Processor Module (R)

READ8_MEMBER(vp415_state::drive_i8155_pb_r)
{
	uint8_t ret = I8155PB_FRLOCK | m_drive_2ppr;
	if (m_drive_rad_mir_dac >= 0x7e && m_drive_rad_mir_dac < 0x82 && BIT(m_drive_i8255_pb, I8255PB_RLS_N_BIT))
		ret |= I8155PB_RAD_MIR;
	logerror("%s: drive_i8155_pb_r: %02x\n", machine().describe_context(), ret);
	return ret;
}

READ8_MEMBER(vp415_state::drive_i8155_pc_r)
{
	logerror("%s: drive_i8155_pc_r: %02x\n", machine().describe_context(), 0);
	return 0;
}

WRITE8_MEMBER(vp415_state::drive_i8255_pa_w)
{
	logerror("%s: drive_i8255_pa_w: radial mirror DAC = %02x\n", machine().describe_context(), data);
	m_drive_rad_mir_dac = data;
}

WRITE8_MEMBER(vp415_state::drive_i8255_pb_w)
{
	m_drive_i8255_pb = data;
	logerror("%s: drive_i8255_pb_w: COMM-1:%d, COMM-2:%d, COMM-3:%d, COMM-4:%d, /RLS:%d, SL-PWR:%d, /RAD-FS:%d, STR1:%d\n"
		, machine().describe_context()
		, BIT(data, I8255PB_COMM1_BIT)
		, BIT(data, I8255PB_COMM2_BIT)
		, BIT(data, I8255PB_COMM3_BIT)
		, BIT(data, I8255PB_COMM4_BIT)
		, BIT(data, I8255PB_RLS_N_BIT)
		, BIT(data, I8255PB_SL_PWR_BIT)
		, BIT(data, I8255PB_RAD_FS_N_BIT)
		, BIT(data, I8255PB_STR1_BIT));
	if (BIT(data, I8255PB_RLS_N_BIT))
	{

	}
}

READ8_MEMBER(vp415_state::drive_i8255_pc_r)
{
	static int focus_kludge = 250;
	static int motor_kludge = 200;
	logerror("%s: drive_i8255_pc_r: %02x\n", machine().describe_context(), m_drive_pc_bits);
	if (focus_kludge > 0)
	{
		focus_kludge--;
	}
	else
	{
		m_drive_pc_bits &= ~I8255PC_NOT_FOCUSED;
	}
	if (motor_kludge > 0)
	{
		motor_kludge--;
	}
	else
	{
		m_drive_pc_bits |= I8255PC_0RPM_N;
	}
	return m_drive_pc_bits;
}

WRITE8_MEMBER(vp415_state::drive_cpu_port1_w)
{
	uint8_t old = m_drive_p1;
	m_drive_p1 = data;
	if ((m_drive_p1 ^ old) & 0xfb) // Ignore bit 2 when logging (LDI)
	{
		logerror("%s: drive_cpu_port1_w: TP2:%d /STR0:%d /STB:%d TX:%d /ATN:%d LDI:%d CP2:%d CP1:%d\n", machine().describe_context()
			, BIT(data, DRIVE_P1_TP2_BIT)
			, BIT(data, DRIVE_P1_STR0_N_BIT)
			, BIT(data, DRIVE_P1_STB_N_BIT)
			, BIT(data, DRIVE_P1_TX_BIT)
			, BIT(data, DRIVE_P1_ATN_N_BIT)
			, BIT(data, DRIVE_P1_LDI_BIT)
			, BIT(data, DRIVE_P1_CP2_BIT)
			, BIT(data, DRIVE_P1_CP1_BIT));
	}
	m_chargen->ldi_w(BIT(data, 2));
}

//READ_LINE_MEMBER(vp415_state::drive_rxd_r)
//{
//	logerror("%s: drive_rxd_r: %d\n", machine().describe_context(), 0);
//	return 0;
//}

//WRITE_LINE_MEMBER(vp415_state::drive_txd_w)
//{
//	logerror("%s: drive_txd_w: %d\n", machine().describe_context(), state);
//}
WRITE8_MEMBER(vp415_state::drive_cpu_port3_w)
{
	logerror("%s: drive_cpu_port3_w: %02x\n", machine().describe_context(), data);
}

void vp415_state::drive_program_map(address_map &map)
{
	map(0x0000, 0x3fff).rom().region(DRIVE_ROM_TAG, 0);
}

void vp415_state::drive_io_map(address_map &map)
{
	map(0x0000, 0x0003).mirror(0xfbfc).rw(I8255_TAG, FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x0400, 0x04ff).mirror(0xf800).rw(I8155_TAG, FUNC(i8155_device::memory_r), FUNC(i8155_device::memory_w));
	map(0x0500, 0x0507).mirror(0xf8f8).rw(I8155_TAG, FUNC(i8155_device::io_r), FUNC(i8155_device::io_w));
}

void vp415_state::video_start()
{
}

uint32_t vp415_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	m_chargen->update_bitmap(bitmap, cliprect);
	return 0;
}

static INPUT_PORTS_START( vp415 )
	PORT_START(vp415_state::SWITCHES_TAG)
		PORT_BIT( 0x03, IP_ACTIVE_HIGH, IPT_UNUSED )
		PORT_DIPNAME( 0x04, 0x00, "BRA (unknown purpose)" )
		PORT_DIPSETTING(    0x04, "On" )
		PORT_DIPSETTING(    0x00, "Off" )
		PORT_DIPNAME( 0x08, 0x00, "BRB (unknown purpose)" )
		PORT_DIPSETTING(    0x08, "On" )
		PORT_DIPSETTING(    0x00, "Off" )
		PORT_DIPNAME( 0x10, 0x00, "DUMP (unknown purpose)" )
		PORT_DIPSETTING(    0x10, "On" )
		PORT_DIPSETTING(    0x00, "Off" )
		PORT_DIPNAME( 0x20, 0x00, "Remote Control IR/Euro" )
		PORT_DIPSETTING(    0x20, "IR" )
		PORT_DIPSETTING(    0x00, "Euro" )
		PORT_DIPNAME( 0x40, 0x00, "REPLAY (unknown purpose)" )
		PORT_DIPSETTING(    0x40, "On" )
		PORT_DIPSETTING(    0x00, "Off" )
		PORT_DIPNAME( 0x80, 0x00, "RESI (unknown purpose)" )
		PORT_DIPSETTING(    0x80, "On" )
		PORT_DIPSETTING(    0x00, "Off" )
INPUT_PORTS_END

MACHINE_CONFIG_START(vp415_state::vp415)
	// Module W: CPU Datagrabber
	MCFG_CPU_ADD(Z80CPU_TAG, Z80, XTAL(8'000'000)/2) // 8MHz through a /2 flip-flop divider, per schematic
	MCFG_CPU_PROGRAM_MAP(z80_program_map)
	MCFG_CPU_IO_MAP(z80_io_map)

	MCFG_DEVICE_ADD(SCSI_TAG, NCR5385, XTAL(8'000'000)/2) // Same clock signal as above, per schematic
	MCFG_NCR5385_INT_CB(WRITELINE(vp415_state, cpu_int1_w))

	// Module S: Control
	MCFG_CPU_ADD(CTRLCPU_TAG, I8031, XTAL(11'059'200)) // 11.059MHz, per schematic
	MCFG_MCS51_PORT_P1_OUT_CB(WRITE8(vp415_state, ctrl_cpu_port1_w));
	MCFG_MCS51_PORT_P1_IN_CB(READ8(vp415_state, ctrl_cpu_port1_r));
	MCFG_MCS51_PORT_P3_OUT_CB(WRITE8(vp415_state, ctrl_cpu_port3_w));
	MCFG_MCS51_PORT_P3_IN_CB(READ8(vp415_state, ctrl_cpu_port3_r));
	MCFG_CPU_PROGRAM_MAP(ctrl_program_map)
	MCFG_CPU_IO_MAP(ctrl_io_map)

	// Module R: Drive
	MCFG_CPU_ADD(CTRLMCU_TAG, I8041, XTAL(4'000'000))
	MCFG_MCS48_PORT_P1_IN_CB(READ8(vp415_state, ctrl_mcu_port1_r));
	MCFG_MCS48_PORT_P1_OUT_CB(WRITE8(vp415_state, ctrl_mcu_port1_w));
	MCFG_MCS48_PORT_P2_IN_CB(READ8(vp415_state, ctrl_mcu_port2_r));
	MCFG_MCS48_PORT_P2_OUT_CB(WRITE8(vp415_state, ctrl_mcu_port2_w));
	MCFG_CPU_PROGRAM_MAP(ctrlmcu_program_map)

	MCFG_CPU_ADD(DRIVECPU_TAG, I8031, XTAL(12'000'000)) // 12MHz, per schematic
	MCFG_MCS51_PORT_P1_OUT_CB(WRITE8(vp415_state, drive_cpu_port1_w));
	MCFG_MCS51_PORT_P3_OUT_CB(WRITE8(vp415_state, drive_cpu_port3_w));
	MCFG_CPU_PROGRAM_MAP(drive_program_map)
	MCFG_CPU_IO_MAP(drive_io_map)

	MCFG_DEVICE_ADD(I8155_TAG, I8155, 0)
	MCFG_I8155_OUT_PORTA_CB(DEVWRITE8(CHARGEN_TAG, mb88303_device, da_w))
	MCFG_I8155_IN_PORTB_CB(READ8(vp415_state, drive_i8155_pb_r))
	MCFG_I8155_IN_PORTC_CB(READ8(vp415_state, drive_i8155_pc_r))

	MCFG_DEVICE_ADD(I8255_TAG, I8255, 0)
	MCFG_I8255_OUT_PORTA_CB(WRITE8(vp415_state, drive_i8255_pa_w))
	MCFG_I8255_OUT_PORTB_CB(WRITE8(vp415_state, drive_i8255_pb_w))
	MCFG_I8255_IN_PORTC_CB(READ8(vp415_state, drive_i8255_pc_r))

	MCFG_DEVICE_ADD(CHARGEN_TAG, MB88303, 0)

	MCFG_DEVICE_ADD(SYNCGEN_TAG, SAA1043, XTAL(5'000'000))
	MCFG_SAA1043_V2_CALLBACK(WRITELINE(vp415_state, refv_w))

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_UPDATE_DRIVER(vp415_state, screen_update)
	MCFG_SCREEN_SIZE(320, 240)
	MCFG_SCREEN_VISIBLE_AREA(0, 319, 0, 239)
MACHINE_CONFIG_END

ROM_START(vp415)
	/* Module R */
	ROM_REGION(0x4000, vp415_state::DRIVE_ROM_TAG, 0) // Version 1.7
	ROM_LOAD( "r.3104 103 6803.6_drive.ic4", 0x0000, 0x4000, CRC(02e4273e) SHA1(198f7ac8f2a880f38046c9d9075ce32f3b730bd4) )

	/* Module S */
	ROM_REGION(0x10000, vp415_state::CONTROL_ROM_TAG, 0) // Version 1.8
	ROM_LOAD( "s.3104 103 6804.9_control.ic2", 0x0000, 0x10000, CRC(10564765) SHA1(8eb6cff7ca7cbfcb3db8b04b697cdd7e364be805) )

	ROM_REGION(0x400, vp415_state::CTRLMCU_TAG, 0)
	ROM_LOAD( "d8041ahc 152.ic11", 0x000, 0x400, CRC(02b6a9b2) SHA1(4123690c3fb4cc987c2ea2dd878e39c8bf0cb4ea) )

	/* Module W */
	ROM_REGION(0x8000, vp415_state::Z80CPU_TAG, 0)
	ROM_LOAD( "w.3104 103 6805.3_cpu", 0x0000, 0x4000, CRC(c2cf4f25) SHA1(e55e1ac917958eb42244bff17a0016b74627c8fa) ) // Version 1.3
	ROM_LOAD( "w.3104 103 6806.3_cpu", 0x4000, 0x4000, CRC(14a45ea0) SHA1(fa028d01094be91e3480c9ad35d46b5546f9ff0f) ) // Version 1.4

	ROM_REGION(0x4000, vp415_state::DESCRAMBLE_ROM_TAG, 0)
	ROM_LOAD( "w.3104 103 6807.0_cpu", 0x0000, 0x4000, CRC(19c2bc87) SHA1(152f8c645588be9fc0dbc840368ab33a13a04e62) ) // Version 1.0

	ROM_REGION(0x4000, vp415_state::SYNC_ROM_TAG, 0)
	ROM_LOAD( "w.3104 103 6808.0_cpu", 0x0000, 0x4000, CRC(bdb601e0) SHA1(4f769aa62b756b157ba9ac8d3ae8bd1228821ff9) ) // Version 1.0
ROM_END

CONS( 1983, vp415, 0, 0, vp415, vp415, vp415_state, 0, "Philips", "VP415", MACHINE_IS_SKELETON )
