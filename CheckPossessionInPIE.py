import socket
import json

def send_python_command(code):
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(10)
        sock.connect(("127.0.0.1", 55557))
        
        command = {
            "type": "execute_python",
            "params": {"code": code}
        }
        
        sock.sendall(json.dumps(command).encode('utf-8'))
        response = sock.recv(8192).decode('utf-8')
        sock.close()
        
        return json.loads(response)
    except Exception as e:
        print(f"Error: {e}")
        return None

# Check possession - FIXED version
check_code = """
import unreal

def log_info(message):
    unreal.log(f"[POSSESSION CHECK] {message}")

log_info("=" * 80)
log_info("Checking Active Possession in PIE")
log_info("=" * 80)

try:
    # Get game world (PIE world if running)
    world = unreal.EditorLevelLibrary.get_game_world()
    
    if world:
        log_info(f"✓ World found: {world.get_name()}")
        
        # Get player controller
        player_controller = unreal.GameplayStatics.get_player_controller(world, 0)
        
        if player_controller:
            log_info(f"✓ Player Controller: {player_controller.get_name()}")
            log_info(f"  Class: {player_controller.get_class().get_name()}")
            
            # Get possessed pawn
            pawn = player_controller.get_controlled_pawn()
            
            if pawn:
                pawn_class = pawn.get_class().get_name()
                pawn_name = pawn.get_name()
                pawn_location = pawn.get_actor_location()
                
                log_info("")
                log_info("✓✓✓ POSSESSION STATUS:")
                log_info(f"  Pawn Name: {pawn_name}")
                log_info(f"  Pawn Class: {pawn_class}")
                log_info(f"  Location: X={pawn_location.x:.1f}, Y={pawn_location.y:.1f}, Z={pawn_location.z:.1f}")
                log_info("")
                
                # Determine what we're possessed into
                if 'BP_Import' in pawn_class:
                    log_info("✓✓✓ SUCCESS! You ARE possessed into BP_Import!")
                    log_info("    If you can't see anything, the ship needs a Camera component")
                    log_info("    If controls don't work, check Auto Receive Input setting")
                elif 'Spaceship' in pawn_class:
                    log_info("✓ Possessed into a Spaceship (but not BP_Import)")
                    log_info(f"  Actual class: {pawn_class}")
                elif 'DefaultPawn' in pawn_class:
                    log_info("✗ WRONG! Possessed into DefaultPawn instead of BP_Import")
                    log_info("  This means BP_TestGameMode DefaultPawnClass is not set correctly")
                else:
                    log_info(f"? Possessed into unexpected pawn type: {pawn_class}")
                
                log_info("")
                
            else:
                log_info("")
                log_info("✗✗✗ NO PAWN POSSESSED!")
                log_info("  Player Controller exists but has no controlled pawn")
                log_info("  This means possession completely failed")
                log_info("")
                log_info("Possible causes:")
                log_info("  1. BP_Import Auto Possess Player = DISABLED (should be PLAYER0)")
                log_info("  2. GameMode failed to spawn any pawn")
                log_info("  3. Pawn was spawned but possession was blocked")
                log_info("")
        else:
            log_info("✗ No Player Controller found in world")
            log_info("  This shouldn't happen - PIE should always have a player controller")
    else:
        log_info("✗ Cannot get game world - PIE may not be running")
        log_info("  Start PIE with Alt+P first, then run this check again")
    
    log_info("=" * 80)
    
except Exception as e:
    log_info(f"Check failed: {e}")
    import traceback
    log_info(traceback.format_exc())
"""

print("=" * 60)
print("CHECKING POSSESSION IN PIE")
print("=" * 60)
print("Make sure PIE is running (Alt+P) before running this check")
print()

result = send_python_command(check_code)

if result and result.get("status") == "success":
    print("✅ Check complete!")
    print()
    print("Check the Unreal Editor Output Log for results:")
    print("  Look for [POSSESSION CHECK] messages")
    print()
    print("Possible results:")
    print("  ✓✓✓ SUCCESS! = You're possessed into BP_Import")
    print("  ✗ WRONG! = Possessed into wrong pawn (DefaultPawn, etc)")
    print("  ✗✗✗ NO PAWN = Not possessed at all (possession failed)")
else:
    print(f"❌ Failed: {result}")
