#pragma once

/* Genode includes */
#include <base/heap.h>
#include <os/ring_buffer.h>
#include <usb_session/connection.h>
#include <usb_session/usb_session.h>
#include <usb_session/device.h>
#include <util/string.h>


namespace Uart
{
	using namespace Genode;

	class Command;
	class Usb_option;
}


class Uart::Command : public String<256>
{
	public:

		template <typename T, typename... TAIL>
		Command(T const &arg, TAIL &&... args) :
			String<256> { arg, args... }
		{ }

		Command() :
			String<256> { }
		{ }

		bool complete() const
		{
			if (length() > 3) {
				auto *str { string() };

				/* length()-1 is always '\0' */
				char last { str[length()-2] };
				char prev { str[length()-3] };

				return last == '\n' && prev == '\r';
			}

			return false;
		}
};


class Uart::Usb_option
{
	private:

		enum State {
			ALT_SETTING,
			RESET,
			INIT,
			READY,
			BUSY,
		} _state { ALT_SETTING };

		enum {
			RING_BUFFER_SIZE  = 4096,         /* size of the ring buffer */
			USB_BUFFER_SIZE   = 512 * 1024,   /* size of the USB cummunication buffer */
			MAX_PACKET_SIZE   = 256,          /* max packet size for bulk request */
			ALT_INTERFACE_NO  = 3,            /* we need the third serial device */
		};

		using Ring_buffer = Genode::Ring_buffer<char, RING_BUFFER_SIZE, Ring_buffer_unsynchronized>;

		Env                       &_env;
		Heap                       _heap             { _env.ram(), _env.rm() };
		Ring_buffer                _ring_buffer      { };
		Allocator_avl              _alloc            { &_heap };
		Signal_context_capability  _connected_sigh   { };
		Signal_context_capability  _read_avail_sigh  { };
		Signal_handler<Usb_option> _state_handler    { _env.ep(),
		                                               *this,
		                                               &Usb_option::_handle_state_change };

		Signal_handler<Usb_option> _io_handler { _env.ep(),
		                                         *this,
		                                         &Usb_option::_handle_io};

		using Device_urb    = Usb::Device::Urb;
		using Interface_urb = Usb::Interface::Urb;
		using Packet_desc   = Usb::Device::Packet_descriptor;

		static constexpr size_t PACKET_STREAM_BUF_SIZE = 2 * (1UL << 20);

		Usb::Connection _connection { _env };
		Usb::Device     _device     { _connection, _alloc, _env.rm() };
		//Usb::Interface  _interface  { _device, Usb::Interface::Index { 0x2, 0x0 }, USB_BUFFER_SIZE };
		Usb::Interface  _interface  { _device, Usb::Interface::Index { ALT_INTERFACE_NO, 0x0}, PACKET_STREAM_BUF_SIZE };

		//Usb::Endpoint _ep_in  { 0x1, 0x2 };
		//Usb::Endpoint _ep_out { 0x2,  0x2 };
		Usb::Endpoint _ep_in  { 0x86, 0x2 };
		Usb::Endpoint _ep_out { 0x4,  0x2 };
		//Usb::Endpoint _ep_in  { 0x84, 0x2 };
		//Usb::Endpoint _ep_out { 0x3,  0x2 };

//		Usb::Endpoint _ep_in  { _interface, Usb::Endpoint::Direction::IN,
//		                        Usb::Endpoint::Type::BULK };
//		Usb::Endpoint _ep_out { _interface, Usb::Endpoint::Direction::OUT,
//		                        Usb::Endpoint::Type::BULK };

		Usb::Interface::Alt_setting _alt_settings { _device, _interface };

		Genode::Constructible<Usb::Interface::Urb> urb { };

		struct Context {
			Usb::Interface &interface;
		} _ctx { _interface };

		struct Comand : Usb::Interface::Urb
		{
			Context             &_ctx;
			Genode::String<256>  _cmd;
			bool                 _completed { false };

			bool completed() const { return _completed; }

			Comand(Context &ctx, Usb::Endpoint &end_point, Genode::String<256> cmd):
				Usb::Interface::Urb(ctx.interface,
				                    end_point,
				                    Usb::Interface::Packet_descriptor::BULK,
				                    0x2),
				_ctx { ctx },
				_cmd { cmd }
			{
				Genode::log("(new) Comand::Comand(): cmd=",
				            "AT",
				           " length=", _cmd.length(),
				           " end_point.valid=", end_point.valid(),
				           " end_point.number=", end_point.number(),
				           " end_point.type=", end_point.type() == 0x2 ? "BULK" : "OTHER",
				           " end_point.direction=", end_point.direction() == 0x0 ? "OUT" : "IN");
				process();
			}

			Comand(Context &ctx, Usb::Endpoint &end_point):
				Usb::Interface::Urb(ctx.interface,
				                    end_point,
				                    Usb::Interface::Packet_descriptor::BULK,
				                    0x0),
				_ctx { ctx },
				_cmd { "" }
			{
				Genode::log("(new stupid empty cmd) Comand::Comand(): cmd=",
				           " length=", 0,
				           " end_point.valid=", end_point.valid(),
				           " end_point.number=", end_point.number(),
				           " end_point.type=", end_point.type() == 0x2 ? "BULK" : "OTHER",
				           " end_point.direction=", end_point.direction() == 0x0 ? "OUT" : "IN");
				process();
			}

			void complete(Usb::Interface::Packet_descriptor::Return_value value)
			{
				Genode::log("Completed with return_value=", Genode::Hex { value });
				_completed = true;
				switch(value) {
					case Packet_desc::OK:        Genode::log("Device Present"); break;
					case Packet_desc::NO_DEVICE: Genode::log("no device");      return;
					default: break;
				};
			}

			void process()
			{
				Genode::log("Command::process(): cmd=", _cmd);
				_ctx.interface.update_urbs<Interface_urb>(
					[this] (Urb &, Byte_range_ptr &/* dst */) {
						Genode::log("produce content");
					},
					[this] (Urb &, Const_byte_range_ptr &/* src */) {
						Genode::log("consume result");
					},
					[this] (Urb &, Usb::Interface::Packet_descriptor::Return_value value) {
						complete(value);
					});
			}
		};

		Genode::Constructible<Comand> _cmd  {};
		Genode::Constructible<Comand> _read {};

		void completed(Packet_desc::Return_value value)
		{
Genode::log("(sigh) Completed with return_value=", Genode::Hex { value });

			switch(value) {
				case Packet_desc::OK: Genode::log("(info) device Present"); break;
				case Packet_desc::NO_DEVICE: Genode::log("(info) no device"); return;
				default: break;
			};

			if (_state == RESET) {
				_reset_req.destruct();
				_state = READY;
			}
			if (_state == ALT_SETTING) {
				_state = INIT;
				_read.construct(_ctx, _ep_in);
			}
			if (_state == READY) {
				if (_connected_sigh.valid())
Genode::log("(sigh) Device is here, opening TERMINAL session");
					Signal_transmitter(_connected_sigh).submit();
			}
		}

		String<32> state_to_string() {
			switch(_state) {
				case ALT_SETTING: return "ALT_SETTING";
				case RESET: return "RESET";
				case INIT: return "INIT";
				case BUSY: return "BUSY";
				default: return "";
			};
		}

		void _handle_io()
		{
Genode::log("(sigh) ", __FUNCTION__, "::", __LINE__, " state=", state_to_string());
			auto out = [] (Device_urb&, Byte_range_ptr&) { Genode::log("(sigh) _handle_io::out()"); };
			auto in  = [] (Device_urb&, Const_byte_range_ptr &br) { Genode::log("(sigh) _handle_io::in() size=", br.num_bytes); };
			auto cpl = [this] (Device_urb&, Packet_desc::Return_value value) { completed(value); };

			switch(_state) {
				case ALT_SETTING: [[fallthrough]];
				case RESET: _device.update_urbs<Device_urb>(out, in, cpl); break;
				case INIT: [[fallthrough]];
				case BUSY:
					if (_cmd.constructed()) {
						_cmd->process();
					}
					if (_read.constructed()) {
						_read->process();
					}
				break;
				default: break;
			};

			if (_cmd.constructed() && _cmd->completed()) {
				_cmd.destruct();
			}
			if (_read.constructed() && _read->completed()) {
				_read.destruct();
				if (_state == INIT) {
Genode::log("(info) state=READY");
Genode::log("(sigh) Device is here, opening TERMINAL session");
					Signal_transmitter(_connected_sigh).submit();
				}
				_state = BUSY;
			}
		}

	public:

		bool ready() const { return _state == READY; }

		size_t write(Command const &cmd)
		{
			if (!_cmd.constructed()) {
				_cmd.construct(_ctx, _ep_out, cmd);
			} else {
				for (unsigned i = 0; i < cmd.length(); ++i) {
					Genode::log("Discarding=", Genode::Hex{cmd.string()[i]});
				}
			}
			return cmd.length();
		}

		Usb_option(Env &env):
			_env { env }
		{
			_device.sigh(_io_handler);
			_interface.sigh(_io_handler);

			_connection.with_xml([] (auto &xml) { Genode::log(xml); });
			_handle_io();
		}

		bool char_avail() const
		{
			return !_ring_buffer.empty();
		}

		char get_char()
		{
			return _ring_buffer.get();
		}

		void connected_sigh(Signal_context_capability sigh)
		{
			_connected_sigh = sigh;
		}

		void read_avail_sigh(Signal_context_capability sigh)
		{
			_read_avail_sigh = sigh;
		}

};
