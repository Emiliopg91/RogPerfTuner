/**
 * Class for utilities
 */
export class Utils {
    /**
     * Private constructor to prevent instantiation
     */
    private constructor() { }

    /**
     * Generate id
     * @param existingIds - List of existing ids to prevent collision
     * @returns Generated id
     */
    public static generateId(existingIds: Array<string> = []): string {
        let id = String(Date.now())

        if (!existingIds.includes(id)) {
            return id;
        }

        let idx: number = 0;
        do {
            if (!existingIds.includes(id + "-" + idx)) {
                return id + "-" + idx;
            }
            idx++;
        } while (true);
    }

    public static intToIp(ipInt: number): string {
        return ((ipInt >>> 24) + '.' + (ipInt >> 16 & 255) + '.' + (ipInt >> 8 & 255) + '.' + (ipInt & 255));
    }

    public static ipAndMaskToSubnet(ipInt: number, netmask: number):string {
        return ((ipInt >>> 24 & netmask >>> 24) + '.' + ((ipInt >> 16 & 255) & (netmask >> 16 & 255)) + '.' + ((ipInt >> 8 & 255) & (netmask >> 8 & 255)) + '.' + ((ipInt & 255) & (netmask & 255)));
    }
}